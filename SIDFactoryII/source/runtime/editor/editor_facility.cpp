#include "runtime/editor/editor_facility.h"
#include "runtime/editor/utilities/import_utils.h"
#include "runtime/environmentdefines.h"
#include "runtime/emulation/cpumos6510.h"
#include "runtime/emulation/cpumemory.h"
#include "runtime/emulation/sid/sidproxy.h"
#include "runtime/execution/executionhandler.h"
#include "runtime/execution/flightrecorder.h"
#include "runtime/editor/converters/converterbase.h"
#include "runtime/editor/screens/screen_base.h"
#include "runtime/editor/screens/screen_intro.h"
#include "runtime/editor/screens/screen_edit.h"
#include "runtime/editor/screens/screen_disk.h"
#include "runtime/editor/screens/screen_convert.h"
#include "runtime/editor/screens/screen_edit_utils.h"
#include "runtime/editor/utilities/editor_utils.h"
#include "runtime/editor/auxilarydata/auxilary_data_collection.h"
#include "runtime/editor/auxilarydata/auxilary_data_hardware_preferences.h"
#include "runtime/editor/editor_types.h"
#include "runtime/editor/dialog/dialog_message.h"
#include "runtime/editor/dialog/dialog_message_yesno.h"
#include "runtime/editor/dialog/dialog_sid_file_info.h"
#include "runtime/editor/driver/driver_utils.h"
#include "runtime/editor/driver/driver_info.h"
#include "runtime/editor/packer/packer.h"
#include "runtime/editor/overlay_control.h"
#include "runtime/editor/keys/keyhook_setup.h"
#include "foundation/graphics/viewport.h"
#include "foundation/graphics/textfield.h"
#include "foundation/platform/iplatform.h"
#include "foundation/input/keyboard.h"
#include "foundation/sound/audiostream.h"
#include "utils/utilities.h"
#include "utils/configfile.h"
#include "utils/config/configtypes.h"
#include "utils/config/configcolors.h"
#include "utils/c64file.h"
#include "utils/psidfile.h"
#include "libraries/ghc/fs_std.h"

// Converter
#include "runtime/editor/converters/jch/converter_jch.h"
#include "runtime/editor/converters/gt/converter_gt.h"
#include "runtime/editor/converters/cc/converter_cc.h"
#include "runtime/editor/converters/mod/converter_mod.h"
#include "runtime/editor/converters/null/converter_null.h"

// System
#include "foundation/base/assert.h"

using namespace Foundation;
using namespace Emulation;
using namespace Utility;
using namespace Utility::Config;
using namespace fs;

namespace Editor
{
    const unsigned int EditorFacility::DefaultDialogWidth = 100;

	EditorFacility::EditorFacility(IPlatform* inPlatform, Viewport* inViewport, Utility::ConfigFile& inConfigFile)
		: m_Viewport(inViewport)
		, m_Platform(inPlatform)
		, m_ConfigFile(inConfigFile)
		, m_IsDone(false)
		, m_CurrentScreen(nullptr)
		, m_RequestedScreen(nullptr)
		, m_FlipOverlayState(false)
		, m_SelectedColorScheme(0)
	{
		// Key setup
		m_KeyHookSetup.ApplyConfigSettings(inConfigFile);

		// Configure colors
		auto color_scheme_names = GetConfigurationValues<ConfigValueString>(inConfigFile, "ColorScheme.Name", {});
		auto color_scheme_filenames = GetConfigurationValues<ConfigValueString>(inConfigFile, "ColorScheme.Filename", {});

		if (color_scheme_names.size() == color_scheme_filenames.size())
		{
			m_ColorSchemeCount = color_scheme_names.size();
			m_SelectedColorScheme = GetSingleConfigurationValue<ConfigValueInt>(inConfigFile, "ColorScheme.Selection", 0);

			ConfigureColorsFromScheme(m_SelectedColorScheme, inConfigFile, *inViewport);
		}

		const bool sequence_highlighting = GetSingleConfigurationValue<ConfigValueInt>(inConfigFile, "Editor.Sequence.Highlights", 0) != 0;
		m_EditState.SetSequenceHighlighting(sequence_highlighting);

		// Create emulation environment
		SIDConfiguration sid_configuration;										// Default settings are applicable

		const bool sid_use_resample = GetSingleConfigurationValue<ConfigValueInt>(inConfigFile, "Sound.Emulation.Resample", 1) != 0;
		sid_configuration.m_eSampleMethod = sid_use_resample ? SID_SAMPLE_METHOD_RESAMPLE_INTERPOLATE : SID_SAMPLE_METHOD_INTERPOLATE;
		sid_configuration.m_eModel = SID_MODEL_8580;

		m_SIDProxy = new SIDProxy(sid_configuration);
		m_CPUMemory = new CPUMemory(0x10000, m_Platform);
		m_CPU = new CPUmos6510();
		m_FlightRecorder = new FlightRecorder(m_Platform, 0x800);
		m_ExecutionHandler = new ExecutionHandler(m_Platform, m_CPU, m_CPUMemory, m_SIDProxy, m_FlightRecorder, inConfigFile);

		// Create audio stream
		const int audio_buffer_size = GetSingleConfigurationValue<ConfigValueInt>(inConfigFile, "Sound.Buffer.Size", 256);
		m_AudioStream = new AudioStream(44100, 16, std::max<const int>(audio_buffer_size, 0x80), m_ExecutionHandler);

		// Create the main text field
		m_TextField = m_Viewport->CreateTextField(m_Viewport->GetClientWidth() / TextField::font_width, m_Viewport->GetClientHeight() / TextField::font_height, 0, 0);
		m_TextField->SetEnable(true);

		// Allocate empty Driver Info class
		m_DriverInfo = std::make_shared<DriverInfo>();

		// Create overlay control
		m_OverlayControl = std::make_unique<OverlayControl>(inConfigFile, inViewport, inPlatform);

		// Create screens
		m_IntroScreen = std::make_unique<ScreenIntro>(
			m_Viewport,
			m_TextField,
			&m_CursorControl,
			m_DisplayState,
			m_KeyHookSetup.GetKeyHookStore(),
			m_DriverInfo,
			[&]() { OnExitIntroScreen(); },
			[&]() { OnExitIntroScreenToLoad(); }
		);

		m_DiskScreen = std::make_unique<ScreenDisk>(
			m_Platform,
			m_Viewport,
			m_TextField,
			&m_CursorControl,
			m_DisplayState,
			m_KeyHookSetup.GetKeyHookStore(),
			m_ConfigFile,
			[&](const std::string& inFilenameSelection, FileType inSaveFileType) { OnFilenameSelection(m_DiskScreen.get(), inFilenameSelection, inSaveFileType); },
			[&]() { OnCancelScreen(m_DiskScreen.get()); });

		m_ConvertScreen = std::make_unique<ScreenConvert>(
			m_Viewport,
			m_TextField,
			&m_CursorControl,
			m_DisplayState,
			m_KeyHookSetup.GetKeyHookStore(),
			m_Platform,
			[&]() { SetCurrentScreen(m_EditScreen.get()); },
			[&](ScreenBase* inCallerScreen, const std::string& inPathAndFilename, std::shared_ptr<Utility::C64File> inConversionResult) { return OnConversionSuccess(inCallerScreen, inPathAndFilename, inConversionResult); }
		);

		// 	bool EditorFacility::OnConversionSuccess(ScreenBase* inCallerScreen, const std::string& inPathAndFilename, std::shared_ptr<Utility::C64File> inConversionResult)


		m_EditScreen = std::make_unique<ScreenEdit>(
			m_Viewport,
			m_TextField,
			&m_CursorControl,
			m_DisplayState,
			m_KeyHookSetup.GetKeyHookStore(),
			m_EditState,
			m_CPUMemory,
			m_ExecutionHandler,
			m_SIDProxy,
			m_DriverInfo,
			[&]() {	m_DiskScreen->SetMode(ScreenDisk::Load); RequestScreen(m_DiskScreen.get()); },
			[&]() {	m_DiskScreen->SetMode(ScreenDisk::Save); m_DiskScreen->SetSuggestedFileName(m_LastSF2PathAndFilename);  RequestScreen(m_DiskScreen.get()); },
			[&]() { m_DiskScreen->SetMode(ScreenDisk::Import); RequestScreen(m_DiskScreen.get()); },
			[&]() {	m_DiskScreen->SetMode(ScreenDisk::LoadInstrument); RequestScreen(m_DiskScreen.get()); },
			[&]() {	m_DiskScreen->SetMode(ScreenDisk::SaveInstrument); m_DiskScreen->SetSuggestedFileName(m_LastSF2PathAndFilename);  RequestScreen(m_DiskScreen.get()); },
			[&]() { OnQuickSave(m_EditScreen.get()); },
			[&](unsigned short inDestinationAddress) { OnPack(m_EditScreen.get(), inDestinationAddress); },
			[&]() { m_FlipOverlayState = true; },
			[&](unsigned int inReconfigureOption) { Reconfigure(inReconfigureOption); }
		);

		// Apply additional configuration to the edit screen
		m_EditScreen->SetAdditionalConfiguration
		(
			GetSingleConfigurationValue<ConfigValueInt>(m_ConfigFile, "Editor.Driver.ConvertLegacyColors", 0) != 0
		);
	}

	EditorFacility::~EditorFacility()
	{
		m_AudioStream->Stop();

		m_Viewport->Destroy(m_TextField);

		delete m_AudioStream;
		delete m_ExecutionHandler;
		delete m_FlightRecorder;
		delete m_SIDProxy;
		delete m_CPU;
		delete m_CPUMemory;
	}

	//--------------------------------------------------------------------------------

	void EditorFacility::Start(const char* inFileToLoad)
	{
		FOUNDATION_ASSERT(m_ExecutionHandler != nullptr);
		FOUNDATION_ASSERT(m_AudioStream != nullptr);

		const bool file_loaded_successfully = [&]()
		{
			if (inFileToLoad != nullptr)
			{
				std::string file_to_load(inFileToLoad);
				return LoadFile(file_to_load);
			}

			return false;
		}();

		// Try to load the driver directly
		if (!file_loaded_successfully)
		{
			std::string default_driver_filename = GetSingleConfigurationValue<ConfigValueString>(m_ConfigFile, "Editor.Driver.Default", std::string("sf2driver11_03.prg"));
			std::string drivers_folder = m_Platform->Storage_GetDriversHomePath();
			LoadFile(drivers_folder + default_driver_filename);
		}

        // After loading, set the current path, so that opening the disk menu will be correct.
        std::error_code ec;
        fs::current_path(m_Platform->Storage_GetHomePath(), ec);
        
        // Start the intro screen
		if(GetSingleConfigurationValue<ConfigValueInt>(m_ConfigFile, "Editor.Skip.Intro", 0) == 0)
			SetCurrentScreen(m_IntroScreen.get());
		else
		{
			if (m_DriverInfo->IsValid())
				SetCurrentScreen(m_EditScreen.get());
			else
			{
				m_DiskScreen->SetMode(ScreenDisk::Mode::Load);
				SetCurrentScreen(m_DiskScreen.get());
			}
		}

		m_AudioStream->Start();
		m_ExecutionHandler->Start();
	}

	void EditorFacility::Stop()
	{
		FOUNDATION_ASSERT(m_AudioStream != nullptr);

		m_AudioStream->Stop();
	}

	//--------------------------------------------------------------------------------

	void EditorFacility::Update(const Keyboard& inKeyboard, const Mouse& inMouse, int inDeltaTicks)
	{
		if (m_IsDone)
			return;

		// Check screen status
		HandleScreenState();

		// Handle component updates
		if (m_CurrentScreen != nullptr)
		{
			m_CurrentScreen->ConsumeInput(inKeyboard, inMouse);
			m_CurrentScreen->Update(inDeltaTicks);
		}

		// Handle overlay flip
		UpdateOverlayEnableDisable();

		// Update overlay
		m_OverlayControl->Update(inDeltaTicks);

		// Update cursor control
		m_CursorControl.Update(inDeltaTicks);

		// Handle viewport updates
		m_Viewport->Begin();

		if (m_CurrentScreen != nullptr)
			m_CurrentScreen->Refresh();

		m_Viewport->End();
	}

	//------------------------------------------------------------------------------------------------------------

	bool EditorFacility::IsDone() const
	{
		return m_IsDone;
	}

	void EditorFacility::TryQuit()
	{
		if (m_CurrentScreen != nullptr)
		{
			m_CurrentScreen->TryQuit([&](bool inQuit)
			{
				if (inQuit)
					m_IsDone = true;
			});
		}
	}

	void EditorFacility::TryLoad(const std::string inPathAndFilename)
	{
		if (m_CurrentScreen != nullptr)
		{
			m_CurrentScreen->TryLoad(inPathAndFilename, [&, path_and_filename = inPathAndFilename](bool inQuit)
			{
				if (LoadFile(path_and_filename))
					ForceRequestScreen(m_EditScreen.get());
				else
					m_CurrentScreen->GetComponentsManager().StartDialog(std::make_shared<DialogMessage>("Invalid file", "The selected file is not compatible with SID Factory II.", DefaultDialogWidth, true, []() {}));
			});
		}
	}


	void EditorFacility::OnWindowResized()
	{
		m_OverlayControl->OnWindowResized();
	}


	void EditorFacility::Reconfigure(unsigned int inReconfigureOption)
	{
		if (inReconfigureOption == 0)
		{
			m_ConfigFile.Reload();
			m_KeyHookSetup.Reset();
			m_KeyHookSetup.ApplyConfigSettings(m_ConfigFile);
			ConfigureColorsFromScheme(m_SelectedColorScheme, m_ConfigFile, *m_Viewport);

			m_EditScreen->SetActivationMessage("Reloaded config!");
			ForceRequestScreen(m_EditScreen.get());
		}
		if (inReconfigureOption == 1)
		{
			m_SelectedColorScheme++;
			if (m_SelectedColorScheme >= m_ColorSchemeCount)
				m_SelectedColorScheme = 0;

			std::string selected_color_scheme_name = ConfigureColorsFromScheme(m_SelectedColorScheme, m_ConfigFile, *m_Viewport);

			if (m_CurrentScreen == m_EditScreen.get())
			{
				m_EditScreen->SetActivationMessage("Selected color scheme: " + selected_color_scheme_name);
				ForceRequestScreen(m_EditScreen.get());
			}
		}
		if (inReconfigureOption == 2)
		{
			std::string selected_color_scheme_name = ConfigureColorsFromScheme(m_SelectedColorScheme, m_ConfigFile, *m_Viewport);

			if (m_CurrentScreen == m_EditScreen.get())
			{
				m_EditScreen->SetActivationMessage("Reloaded color scheme: " + selected_color_scheme_name);
				ForceRequestScreen(m_EditScreen.get());
			}
		}
	}


	void EditorFacility::UpdateOverlayEnableDisable()
	{
		if (m_FlipOverlayState)
		{
			FOUNDATION_ASSERT(m_OverlayControl != nullptr);
			m_OverlayControl->SetOverlayEnabled(!m_OverlayControl->GetOverlayEnabled());

			m_FlipOverlayState = false;
		}
	}



	//------------------------------------------------------------------------------------------------------------

	void EditorFacility::RequestScreen(ScreenBase* inRequestedScreen)
	{
		FOUNDATION_ASSERT(m_RequestedScreen == nullptr);

		if(m_CurrentScreen != inRequestedScreen)
			m_RequestedScreen = inRequestedScreen;
	}

	void EditorFacility::ForceRequestScreen(ScreenBase* inRequestedScreen)
	{
		FOUNDATION_ASSERT(m_RequestedScreen == nullptr);
		m_RequestedScreen = inRequestedScreen;
	}

	void EditorFacility::SetCurrentScreen(ScreenBase* inCurrentScreen)
	{
		if (m_CurrentScreen != nullptr)
			m_CurrentScreen->Deactivate();

		m_CurrentScreen = inCurrentScreen;

		if (m_CurrentScreen != nullptr)
			m_CurrentScreen->Activate();
	}

	void EditorFacility::HandleScreenState()
	{
		if (m_RequestedScreen != nullptr)
		{
			SetCurrentScreen(m_RequestedScreen);
			m_RequestedScreen = nullptr;
		}
	}

	//------------------------------------------------------------------------------------------------------------

	bool EditorFacility::IsFileSF2(const std::string& inPathAndFilename)
	{
		// Read test music data to cpu memory
		void* data = nullptr;
		long data_size = 0;

		std::shared_ptr<DriverInfo> driver_info = std::make_shared<DriverInfo>();

		if (Utility::ReadFile(inPathAndFilename, 65536, &data, data_size))
		{
			if (data_size > 2)
			{
				std::shared_ptr<Utility::C64File> c64_file = Utility::C64File::CreateFromPRGData(data, static_cast<unsigned int>(data_size));

				if(c64_file != nullptr)
					driver_info->Parse(*c64_file);
			}

			delete[] static_cast<char*>(data);
		}

		return driver_info->IsValid() || driver_info->IsParticalyValid();
	}


	bool EditorFacility::LoadFile(const std::string& inPathAndFilename)
	{
		const int max_file_size = 0x10000;

		// Read test music data to cpu memory
		void* data = nullptr;
		long data_size = 0;

		std::shared_ptr<DriverInfo> driver_info = std::make_shared<DriverInfo>();
		std::shared_ptr<Utility::C64File> c64_file = nullptr;

		if (Utility::ReadFile(inPathAndFilename, max_file_size, &data, data_size))
		{
			// Try to parse the data immediately
			c64_file = Utility::C64File::CreateFromPRGData(data, static_cast<unsigned int>(data_size));

			if (c64_file != nullptr)
				driver_info->Parse(*c64_file);

			if (driver_info->IsValid())
			{
				m_DriverInfo->GetAuxilaryDataCollection().Reset();
				m_DriverInfo = driver_info;

				// Copy the data to the emulated memory
				m_CPUMemory->Lock();
				m_CPUMemory->Clear();
				m_CPUMemory->SetData(c64_file->GetTopAddress(), c64_file->GetData(), c64_file->GetDataSize());
				m_CPUMemory->Unlock();

				// Init the execution handler 
				m_ExecutionHandler->SetInitVector(m_DriverInfo->GetDriverCommon().m_InitAddress);
				m_ExecutionHandler->SetStopVector(m_DriverInfo->GetDriverCommon().m_StopAddress);
				m_ExecutionHandler->SetUpdateVector(m_DriverInfo->GetDriverCommon().m_UpdateAddress);

				// Store name of last read file
				SetLastSavedPathAndFilename(inPathAndFilename);

				// Flush undo after load
				m_EditScreen->FlushUndo();

				// Notify overlay
				m_OverlayControl->OnChange(*m_DriverInfo);
			}

			delete[] static_cast<char*>(data);
		}

		return driver_info->IsValid();
	}


	bool EditorFacility::LoadFileForImport(const std::string& inPathAndFilename, std::shared_ptr<DriverInfo>& outDriverInfo, std::shared_ptr<Utility::C64File>& outC64File)
	{
		// Read test music data to cpu memory
		void* data = nullptr;
		long data_size = 0;

		outDriverInfo = std::make_shared<DriverInfo>();

		if (Utility::ReadFile(inPathAndFilename, 65536, &data, data_size))
		{
			if (data_size > 2)
			{
				outC64File = Utility::C64File::CreateFromPRGData(data, static_cast<unsigned int>(data_size));
				if (outC64File != nullptr)
				{
					outDriverInfo->Parse(*outC64File);

					// Store name of last read file
					SetLastSavedPathAndFilename(inPathAndFilename);

					// Flush undo after load
					m_EditScreen->FlushUndo();

					return true;
				}
			}

			delete[] static_cast<char*>(data);
		}

		return false;
	}


	bool EditorFacility::LoadAndConvertFile(const std::string& inPathAndFilename, ScreenBase* inCallerScreen, std::function<void()> inSuccesfullConversionAction)
	{
		const int max_file_size = 16 * 1024 * 1024;		// 16MB

		// Read test music data to cpu memory
		void* data = nullptr;
		long data_size = 0;

		auto on_successfull_conversion = [this, inPathAndFilename, inCallerScreen, inSuccesfullConversionAction](std::shared_ptr<Utility::C64File> inC64File)
		{
			std::shared_ptr<DriverInfo> driver_info = std::make_shared<DriverInfo>();

			if (inC64File != nullptr)
			{
				driver_info->Parse(*inC64File);

				if (driver_info->IsValid())
				{
					m_DriverInfo->GetAuxilaryDataCollection().Reset();
					m_DriverInfo = driver_info;

					// Copy the data to the emulated memory
					m_CPUMemory->Lock();
					m_CPUMemory->Clear();
					m_CPUMemory->SetData(inC64File->GetTopAddress(), inC64File->GetData(), inC64File->GetDataSize());
					m_CPUMemory->Unlock();

					// Init the execution handler 
					m_ExecutionHandler->SetInitVector(m_DriverInfo->GetDriverCommon().m_InitAddress);
					m_ExecutionHandler->SetStopVector(m_DriverInfo->GetDriverCommon().m_StopAddress);
					m_ExecutionHandler->SetUpdateVector(m_DriverInfo->GetDriverCommon().m_UpdateAddress);

					// Store name of last read file
					SetLastSavedPathAndFilename(inPathAndFilename);

					// Flush undo after load
					m_EditScreen->FlushUndo();

					// Notify overlay
					m_OverlayControl->OnChange(*m_DriverInfo);

					inSuccesfullConversionAction();
					return;
				}
			}

			inCallerScreen->GetComponentsManager().StartDialog(std::make_shared<DialogMessage>("Conversion failed", "The selected file couldn't be converted correctly.", DefaultDialogWidth, true, []() {}));
		};

		if (Utility::ReadFile(inPathAndFilename, max_file_size, &data, data_size))
		{
			// Try converting, if there're no valid results yet
			if (inCallerScreen != nullptr && data_size > 2)
			{
				auto converters = GetConverters();
				const size_t converter_count = converters.size();

				for (size_t i = 0; i < converter_count; ++i)
				{
					if (converters[i]->CanConvert(data, data_size))
					{
						m_ConvertScreen->PassConverterAndData(inPathAndFilename, converters[i], data, data_size);
						RequestScreen(m_ConvertScreen.get());

						return true;
					}
				}
			}

			delete[] static_cast<char*>(data);
		}

		return false;
	}


	bool EditorFacility::SaveFile(const std::string& inPathAndFilename)
	{
		if (m_DriverInfo->IsValid())
		{
			m_CPUMemory->Lock();

			const unsigned short top_of_file_address = m_DriverInfo->GetTopAddress();
			const unsigned short end_of_file_address = DriverUtils::GetEndOfMusicDataAddress(*m_DriverInfo, reinterpret_cast<const Emulation::IMemoryRandomReadAccess&>(*m_CPUMemory));
			const unsigned short data_size = end_of_file_address - top_of_file_address;

			unsigned char* data = new unsigned char[data_size];
			m_CPUMemory->GetData(top_of_file_address, data, data_size);
			m_CPUMemory->Unlock();
			std::shared_ptr<Utility::C64File> file = Utility::C64File::CreateFromData(top_of_file_address, data, data_size);
			delete[] data;

			Utility::C64FileWriter file_writer(*file, end_of_file_address, true);

			const unsigned short irq_vector = file_writer.GetWriteAddress();
			DriverUtils::InsertIRQ(*m_DriverInfo, file_writer);

			const unsigned short auxilary_data_vector = file_writer.GetWriteAddress();
			m_DriverInfo->GetAuxilaryDataCollection().Save(file_writer);

			// Adjust IRQ and auxilary data vectors in file
			const unsigned short driver_init_vector = m_DriverInfo->GetDriverCommon().m_InitAddress;
			(*file)[driver_init_vector - 2] = static_cast<unsigned char>(irq_vector & 0xff);
			(*file)[driver_init_vector - 1] = static_cast<unsigned char>(irq_vector >> 8);
			(*file)[driver_init_vector - 5] = static_cast<unsigned char>(auxilary_data_vector & 0xff);
			(*file)[driver_init_vector - 4] = static_cast<unsigned char>(auxilary_data_vector >> 8);

			// Save to disk
			if (!Utility::WriteFile(inPathAndFilename, file))
				return false;

			SetLastSavedPathAndFilename(inPathAndFilename);

			return true;
		}

		return false;
	}


	bool EditorFacility::SavePackedFile(const std::string& inFileName)
	{
		if (m_PackedData != nullptr)
		{
			// Create file
			std::shared_ptr<Utility::C64File> file = Utility::C64File::CreateFromData(m_PackedData->GetTopAddress(), m_PackedData->GetData(), static_cast<unsigned short>(m_PackedData->GetDataSize()));

			// Save to disk
			Utility::WriteFile(inFileName, file);

			return true;
		}

		return false;
	}


	bool EditorFacility::SavePackedFileToSID(ScreenBase* inCallerScreen, const std::string& inFileName)
	{
		if (m_PackedData != nullptr)
		{
			auto do_save = [&, inFileName](std::string inTitle, std::string inAuthor, std::string inCopyright)
			{
				unsigned short top_of_file_address = m_PackedData->GetTopAddress();
				unsigned short data_size = static_cast<unsigned short>(m_PackedData->GetDataSize());

				unsigned char* data = new unsigned char[data_size + 2];

				data[0] = static_cast<unsigned char>(top_of_file_address & 0xff);
				data[1] = static_cast<unsigned char>(top_of_file_address >> 8);

				unsigned char* packed_data = m_PackedData->GetData();

				for (int i = 0; i < data_size; ++i)
					data[i + 2] = packed_data[i];

				// Save PSID file to disk, also
				const auto& driver_common = m_DriverInfo->GetDriverCommon();
				const auto& hardware_preferences = m_DriverInfo->GetAuxilaryDataCollection().GetHardwarePreferences();

				Utility::PSIDFile psid_file(
					data,
					data_size + 2,
					0,
					driver_common.m_UpdateAddress - driver_common.m_InitAddress,
					1,
					inTitle,
					inAuthor,
					inCopyright,
					hardware_preferences.GetSIDModel() == AuxilaryDataHardwarePreferences::MOS6581,
					hardware_preferences.GetRegion() == AuxilaryDataHardwarePreferences::PAL);

				const unsigned char* psid_data = psid_file.GetData();

				Utility::WriteFile(inFileName, psid_data, psid_file.GetDataSize());

				delete[] data;

				RequestScreen(m_EditScreen.get());
			};

			inCallerScreen->GetComponentsManager().StartDialog(std::make_shared<DialogSIDFileInfo>(do_save, []() {}));

			return true;
		}

		return false;
	}
	//-------------------------------------------------------------------------------------------------------------------------

	void EditorFacility::OnCancelScreen(ScreenBase* inCallerScreen)
	{
		if (inCallerScreen == m_DiskScreen.get())
		{
			if (m_DriverInfo->IsValid())
				RequestScreen(m_EditScreen.get());
			else
				inCallerScreen->GetComponentsManager().StartDialog(std::make_shared<DialogMessage>("No driver loaded", "Cannot enter the editor when no driver has been loaded. Please load a valid file from the file selection screen!", DefaultDialogWidth, true, []() {}));
		}
	}


	void EditorFacility::OnFilenameSelection(ScreenBase* inCallerScreen, const std::string& inSelectedFilename, FileType inFileType)
	{
		if (inCallerScreen == m_DiskScreen.get())
		{
			// Handle loading
			if (m_DiskScreen->GetMode() == ScreenDisk::Load)
			{
				FOUNDATION_ASSERT(inFileType == FileType::SF2);
				DoLoad(inCallerScreen, inSelectedFilename);
			}

			// Handle saving
			if (m_DiskScreen->GetMode() == ScreenDisk::Save)
			{
				FOUNDATION_ASSERT(inFileType == FileType::SF2);
				DoSave(inCallerScreen, inSelectedFilename);
			}

			// Handle importing
			if (m_DiskScreen->GetMode() == ScreenDisk::Import)
			{
				FOUNDATION_ASSERT(inFileType == FileType::SF2);
				DoImport(inCallerScreen, inSelectedFilename);
			}

			// Handle loading
			if (m_DiskScreen->GetMode() == ScreenDisk::LoadInstrument)
			{
				FOUNDATION_ASSERT(inFileType == FileType::SI2);
				DoLoadInstrument(inCallerScreen, inSelectedFilename);
			}

			// Handle saving
			if (m_DiskScreen->GetMode() == ScreenDisk::SaveInstrument)
			{
				FOUNDATION_ASSERT(inFileType == FileType::SI2);
				DoSaveInstrument(inCallerScreen, inSelectedFilename);
			}

			// Handle save packed
			if (m_DiskScreen->GetMode() == ScreenDisk::SavePacked)
			{
				if (inFileType == FileType::PRG)
					DoSavePacked(inCallerScreen, inSelectedFilename);
				else
				{
					FOUNDATION_ASSERT(inFileType == FileType::SID);
					DoSavePackedToSID(inCallerScreen, inSelectedFilename);
				}
			}
		}
	}


	void EditorFacility::OnExitIntroScreen()
	{
		FOUNDATION_ASSERT(m_DriverInfo != nullptr);

		if (m_DriverInfo->IsValid())
			SetCurrentScreen(m_EditScreen.get());
		else
		{
			// .. otherwise go to the load screen
			m_DiskScreen->SetMode(ScreenDisk::Mode::Load);
			SetCurrentScreen(m_DiskScreen.get());
		}
	}


	void EditorFacility::OnExitIntroScreenToLoad()
	{
		m_DiskScreen->SetMode(ScreenDisk::Mode::Load);
		SetCurrentScreen(m_DiskScreen.get());
	}


	bool EditorFacility::OnConversionSuccess(ScreenBase* inCallerScreen, const std::string& inPathAndFilename, std::shared_ptr<Utility::C64File> inConversionResult)
	{
		std::shared_ptr<DriverInfo> driver_info = std::make_shared<DriverInfo>();

		if (inConversionResult != nullptr)
		{
			driver_info->Parse(*inConversionResult);

			if (driver_info->IsValid())
			{
				m_DriverInfo->GetAuxilaryDataCollection().Reset();
				m_DriverInfo = driver_info;

				// Copy the data to the emulated memory
				m_CPUMemory->Lock();
				m_CPUMemory->Clear();
				m_CPUMemory->SetData(inConversionResult->GetTopAddress(), inConversionResult->GetData(), inConversionResult->GetDataSize());
				m_CPUMemory->Unlock();

				// Init the execution handler 
				m_ExecutionHandler->SetInitVector(m_DriverInfo->GetDriverCommon().m_InitAddress);
				m_ExecutionHandler->SetStopVector(m_DriverInfo->GetDriverCommon().m_StopAddress);
				m_ExecutionHandler->SetUpdateVector(m_DriverInfo->GetDriverCommon().m_UpdateAddress);

				// Store name of last read file
				SetLastSavedPathAndFilename(inPathAndFilename);

				// Flush undo after load
				m_EditScreen->FlushUndo();

				// Notify overlay
				m_OverlayControl->OnChange(*m_DriverInfo);

				return true;
			}
		}

		inCallerScreen->GetComponentsManager().StartDialog(std::make_shared<DialogMessage>("Conversion failed", "The selected file couldn't be converted correctly.", DefaultDialogWidth, true, []() {}));

		return false;
	}


	void EditorFacility::OnPack(ScreenBase* inCallerScreen, unsigned short inDestinationAddress)
	{
		const bool is_uppercase = m_DisplayState.IsHexUppercase();

		Packer packer(*m_CPUMemory, *m_DriverInfo, inDestinationAddress);
		m_PackedData = packer.GetResult();

		std::string packing_info;
		packing_info += "Range: 0x" + EditorUtils::ConvertToHexValue(static_cast<unsigned short>(m_PackedData->GetTopAddress()), is_uppercase) + " - 0x" + EditorUtils::ConvertToHexValue(static_cast<unsigned short>(m_PackedData->GetBottomAddress()), is_uppercase) + "\n";
		packing_info += "Size : 0x" + EditorUtils::ConvertToHexValue(static_cast<unsigned short>(m_PackedData->GetDataSize()), is_uppercase);

		inCallerScreen->GetComponentsManager().StartDialog(
			std::make_shared<DialogMessage>("Packing results", packing_info, 30, false, [&]() 
			{
				m_DiskScreen->SetMode(ScreenDisk::Mode::SavePacked);
				SetCurrentScreen(m_DiskScreen.get());
			})
		);
	}


	void EditorFacility::OnQuickSave(ScreenBase* inCallerScreen)
	{
		DoQuickSave(inCallerScreen);
	}


    void EditorFacility::OnSaveError(ScreenBase* inCallerScreen)
    {
        inCallerScreen->GetComponentsManager().StartDialog(std::make_shared<DialogMessage>("Error", "The file could not be saved to the current destination!", DefaultDialogWidth, true, []() {}));
    }

	//-------------------------------------------------------------------------------------------------------------------------


	void EditorFacility::DoLoad(ScreenBase* inCallerScreen, const std::string& inSelectedFilename)
	{
		auto on_success = [this]()
		{
			RequestScreen(m_EditScreen.get());
		};

		auto do_load = [this, on_success, inSelectedFilename, inCallerScreen]()
		{
			if (LoadFile(inSelectedFilename))
				on_success();
			else
			{
				if (LoadAndConvertFile(inSelectedFilename, inCallerScreen, on_success))
					return;

				inCallerScreen->GetComponentsManager().StartDialog(std::make_shared<DialogMessage>("Invalid file", "The selected file is not compatible with SID Factory II.", DefaultDialogWidth, true, []() {}));
			}
		};

		if (m_DriverInfo->IsValid())
			inCallerScreen->GetComponentsManager().StartDialog(std::make_shared<DialogMessageYesNo>("Load file", "Are you sure you want to load:\n" + inSelectedFilename + "? \nAny unsaved changes will be lost!", DefaultDialogWidth, do_load, []() {}));
		else
			do_load();
	}


	void EditorFacility::DoSave(ScreenBase* inCallerScreen, const std::string& inSelectedFilename)
	{
		path save_path_and_filename = inSelectedFilename;

		if (!exists(save_path_and_filename))
		{
			if(SaveFile(save_path_and_filename.string()))
                RequestScreen(m_EditScreen.get());
            else
                OnSaveError(inCallerScreen);
		}
		else if (IsFileSF2(save_path_and_filename.string()))
		{
			auto do_save = [save_path_and_filename, inCallerScreen, this]()
			{
				if(SaveFile(save_path_and_filename.string()))
                    this->RequestScreen(this->m_EditScreen.get());
                else
                    OnSaveError(inCallerScreen);
			};

			inCallerScreen->GetComponentsManager().StartDialog(std::make_shared<DialogMessageYesNo>("Warning", inSelectedFilename + "\nAlready exists! Are you sure you want to overwrite it?", DefaultDialogWidth, do_save, []() {}));
		}
		else
		{
			inCallerScreen->GetComponentsManager().StartDialog(std::make_shared<DialogMessage>("Illegal save destination", "You are trying to overwrite a file, which cannot be identified as belonging to SID Factory II.\nPlease choose another name!", DefaultDialogWidth, true, []() {}));
		}
	}


	void EditorFacility::DoLoadInstrument(ScreenBase* inCallerScreen, const std::string& inSelectedFilename)
	{
		RequestScreen(m_EditScreen.get());
	}


	void EditorFacility::DoSaveInstrument(ScreenBase* inCallerScreen, const std::string& inSelectedFilename)
	{
		RequestScreen(m_EditScreen.get());
	}


	void EditorFacility::DoQuickSave(ScreenBase* inCallerScreen)
	{
		path save_path_and_filename = m_LastSF2PathAndFilename;

		const bool sf2_extension = save_path_and_filename.extension().string() == ".sf2";

		if (!exists(save_path_and_filename))
			inCallerScreen->GetComponentsManager().StartDialog(std::make_shared<DialogMessage>("Illegal save destination", "The quick save file does not exist!", DefaultDialogWidth, true, []() {}));
		else if (!IsFileSF2(save_path_and_filename.string()))
			inCallerScreen->GetComponentsManager().StartDialog(std::make_shared<DialogMessage>("Illegal save destination", "You are trying to quick save to a file, which cannot be identified as belonging to SID Factory II.\nPlease save through the save disk menu!", DefaultDialogWidth, true, []() {}));
		else if (!sf2_extension)
			inCallerScreen->GetComponentsManager().StartDialog(std::make_shared<DialogMessage>("Illegal save destination", "You are trying to quick save to a file, with an extension other than .sf2.\nPlease save through the save disk menu!", DefaultDialogWidth, true, []() {}));
		else
		{	
			auto do_save = [save_path_and_filename, inCallerScreen, this]()
			{
				if(SaveFile(save_path_and_filename.string()))
                    this->m_EditScreen->SetStatusBarMessage(" Quick saved to: " + save_path_and_filename.filename().string(), 5000);
                else
                    this->OnSaveError(inCallerScreen);
			};

			inCallerScreen->GetComponentsManager().StartDialog(std::make_shared<DialogMessageYesNo>("Warning", "Do you want to perform a quick save to:\n" + save_path_and_filename.string() + "?", DefaultDialogWidth, do_save, []() {}));
		}
	}


	void EditorFacility::DoImport(ScreenBase* inCallerScreen, const std::string& inSelectedFilename)
	{
		auto do_load = [&, inSelectedFilename, inCallerScreen]()
		{
			std::shared_ptr<DriverInfo> import_driver_info = nullptr;
			std::shared_ptr<Utility::C64File> import_c64_file = nullptr;

			if (LoadFileForImport(inSelectedFilename, import_driver_info, import_c64_file))
			{
				const bool valid_tables = import_driver_info->HasParsedHeaderBlock(DriverInfo::HeaderBlockID::ID_DriverTables);
				const bool valid_music_data = import_driver_info->HasParsedHeaderBlock(DriverInfo::HeaderBlockID::ID_MusicData);

				if (!valid_music_data)
					inCallerScreen->GetComponentsManager().StartDialog(std::make_shared<DialogMessage>("Import failure", "Unable to import music data!", DefaultDialogWidth, true, []() {}));
				else if(!valid_tables)
					inCallerScreen->GetComponentsManager().StartDialog(std::make_shared<DialogMessage>("Import failure", "Unable to import table definitions!", DefaultDialogWidth, true, []() {}));
				else 
				{
					ImportUtils::Import(inCallerScreen, *m_DriverInfo, *m_CPUMemory, *import_driver_info, *import_c64_file);
					this->RequestScreen(this->m_EditScreen.get());
				}

			}
			else
				inCallerScreen->GetComponentsManager().StartDialog(std::make_shared<DialogMessage>("Invalid file", "The selected file is not compatible with SID Factory II import.", DefaultDialogWidth, true, []() {}));
		};

		inCallerScreen->GetComponentsManager().StartDialog(std::make_shared<DialogMessageYesNo>("Import song data", "Are you sure you want to import the song data from\n" + inSelectedFilename + "?\nAny unsaved changes will be lost!", DefaultDialogWidth, do_load, []() {}));
	}


	void EditorFacility::DoSavePacked(ScreenBase* inCallerScreen, const std::string& inSelectedFilename)
	{
		path save_path_and_filename = inSelectedFilename;

		if (!exists(save_path_and_filename))
		{
			SavePackedFile(save_path_and_filename.string());
			RequestScreen(m_EditScreen.get());
		}
		else
		{
			auto do_save = [save_path_and_filename, this]()
			{
				SavePackedFile(save_path_and_filename.string());
				this->RequestScreen(this->m_EditScreen.get());
			};

			inCallerScreen->GetComponentsManager().StartDialog(std::make_shared<DialogMessageYesNo>("Warning", inSelectedFilename + "\nAlready exists! Are you sure you want to overwrite it?", DefaultDialogWidth, do_save, []() {}));
		}
	}


	void EditorFacility::DoSavePackedToSID(ScreenBase* inCallerScreen, const std::string& inSelectedFilename)
	{
		path save_path_and_filename = inSelectedFilename;

		if (!exists(save_path_and_filename))
		{
			SavePackedFileToSID(inCallerScreen, save_path_and_filename.string());
		}
		else
		{
			auto do_save = [inCallerScreen, save_path_and_filename, this]()
			{
				SavePackedFileToSID(inCallerScreen, save_path_and_filename.string());
			};

			inCallerScreen->GetComponentsManager().StartDialog(std::make_shared<DialogMessageYesNo>("Warning", inSelectedFilename + "\nAlready exists! Are you sure you want to overwrite it?", DefaultDialogWidth, do_save, []() {}));
		}
	}


	void EditorFacility::SetLastSavedPathAndFilename(const std::string& inLastSavedPathAndFilename)
	{
		m_LastSF2PathAndFilename = inLastSavedPathAndFilename;

		const size_t length = inLastSavedPathAndFilename.size();

		for (size_t i = length - 1; i >= 0; --i)
		{
			const char character = inLastSavedPathAndFilename[i];
			if (character == '/' || character == '\\')
			{
				std::string file_name = inLastSavedPathAndFilename.substr(i + 1, length - (i + 1));
				m_Viewport->SetAdditionTitleInfo(file_name);

				return;
			}
		}

		m_Viewport->SetAdditionTitleInfo("");
	}


	std::string EditorFacility::ConfigureColorsFromScheme(int inSchemeIndex, const Utility::ConfigFile& inMainConfigFile, Foundation::Viewport& inViewport)
	{
		if (inSchemeIndex < m_ColorSchemeCount)
		{
			auto color_scheme_names = GetConfigurationValues<ConfigValueString>(inMainConfigFile, "ColorScheme.Name", {});
			auto color_scheme_filenames = GetConfigurationValues<ConfigValueString>(inMainConfigFile, "ColorScheme.Filename", {});

			std::string color_config_path_and_filename = m_Platform->Storage_GetColorSchemesHomePath() + color_scheme_filenames[inSchemeIndex];
			ConfigFile color_config(*m_Platform, color_config_path_and_filename, inMainConfigFile.GetValidSectionTags());

			if (color_config.IsValid())
			{
				Utility::Config::ConfigureColors(color_config, inViewport);
				return color_scheme_names[inSchemeIndex];
			}
		}

		Utility::Config::ConfigureColors(inMainConfigFile, inViewport);
		return "Default";
	}


	std::vector<std::shared_ptr<ConverterBase>> EditorFacility::GetConverters() const
	{
		std::vector<std::shared_ptr<ConverterBase>> converters;

		// Configure the converters
		converters.push_back(std::make_shared<ConverterJCH>());
		converters.push_back(std::make_shared<ConverterGT>());
		converters.push_back(std::make_shared<ConverterCC>());
		converters.push_back(std::make_shared<ConverterMod>());
		converters.push_back(std::make_shared<ConverterNull>());

		return converters;
	}

}

