#include "runtime/editor/editor_facility.h"
#include "foundation/graphics/textfield.h"
#include "foundation/graphics/viewport.h"
#include "foundation/input/keyboard.h"
#include "foundation/platform/iplatform.h"
#include "foundation/sound/audiostream.h"
#include "libraries/ghc/fs_std.h"
#include "runtime/editor/auxilarydata/auxilary_data_collection.h"
#include "runtime/editor/auxilarydata/auxilary_data_hardware_preferences.h"
#include "runtime/editor/auxilarydata/auxilary_data_songs.h"
#include "runtime/editor/converters/converterbase.h"
#include "runtime/editor/dialog/dialog_message.h"
#include "runtime/editor/dialog/dialog_message_yesno.h"
#include "runtime/editor/dialog/dialog_sid_file_info.h"
#include "runtime/editor/driver/driver_info.h"
#include "runtime/editor/driver/driver_utils.h"
#include "runtime/editor/editor_types.h"
#include "runtime/editor/keys/keyhook_setup.h"
#include "runtime/editor/overlay_control.h"
#include "runtime/editor/packer/packer.h"
#include "runtime/editor/screens/screen_base.h"
#include "runtime/editor/screens/screen_convert.h"
#include "runtime/editor/screens/screen_disk.h"
#include "runtime/editor/screens/screen_edit.h"
#include "runtime/editor/screens/screen_edit_utils.h"
#include "runtime/editor/screens/screen_intro.h"
#include "runtime/editor/utilities/editor_utils.h"
#include "runtime/editor/utilities/import_utils.h"
#include "runtime/editor/datacopy/copypaste.h"
#include "runtime/emulation/cpumemory.h"
#include "runtime/emulation/cpumos6510.h"
#include "runtime/emulation/sid/sidproxy.h"
#include "runtime/environmentdefines.h"
#include "runtime/execution/executionhandler.h"
#include "runtime/execution/flightrecorder.h"
#include "utils/c64file.h"
#include "utils/config/configcolors.h"
#include "utils/config/configtypes.h"
#include "utils/configfile.h"
#include "utils/global.h"
#include "utils/logging.h"
#include "utils/psidfile.h"
#include "utils/utilities.h"

// Converter
#include "runtime/editor/converters/cc/converter_cc.h"
#include "runtime/editor/converters/gt/converter_gt.h"
#include "runtime/editor/converters/jch/converter_jch.h"
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

	EditorFacility::EditorFacility(Viewport* inViewport)
		: m_Viewport(inViewport)
		, m_IsDone(false)
		, m_CurrentScreen(nullptr)
		, m_RequestedScreen(nullptr)
		, m_FlipOverlayState(false)
		, m_SelectedColorScheme(0)
	{

		ConfigFile& config = Global::instance().GetConfig();
		IPlatform& platform = Global::instance().GetPlatform();

		// Key setup
		m_KeyHookSetup.ApplyConfigSettings(config);

		// Configure editor
		auto color_scheme_names = GetConfigurationValues<ConfigValueString>(config, "ColorScheme.Name", {});
		auto color_scheme_filenames = GetConfigurationValues<ConfigValueString>(config, "ColorScheme.Filename", {});

		if (color_scheme_names.size() == color_scheme_filenames.size())
		{
			m_ColorSchemeCount = color_scheme_names.size();
			m_SelectedColorScheme = GetSingleConfigurationValue<ConfigValueInt>(config, "ColorScheme.Selection", 0);

			ConfigureColorsFromScheme(m_SelectedColorScheme, *inViewport);
		}
		else
		{
			Utility::Logging::instance().Error("Number of color scheme names (%d) does not match number of color scheme filenames (%d)", color_scheme_names.size(), color_scheme_filenames.size());
		}

		const bool sequence_highlighting = GetSingleConfigurationValue<ConfigValueInt>(config, "Editor.Sequence.Highlights", 0) != 0;
		m_EditState.SetSequenceHighlighting(sequence_highlighting);
		const bool follow_play = GetSingleConfigurationValue<ConfigValueInt>(config, "Editor.Follow.Play", 0) != 0;
		m_EditState.SetFollowPlayMode(follow_play);

		// Create emulation environment
		SIDConfiguration sid_configuration; // Default settings are applicable

		const bool sid_use_resample = GetSingleConfigurationValue<ConfigValueInt>(config, "Sound.Emulation.Resample", 1) != 0;
		
		int sid_sample_frequency = GetSingleConfigurationValue<ConfigValueInt>(config, "Sound.Emulation.SampleFrequency", 44100);
		if (sid_sample_frequency < 11025)
		{
			// In resampling mode reSID can downsample down to clock/125 Hz. With NTSC this puts us at min. 8200Hz,
			// so let's use 11025 which is the next higher usual rate. 
			Logging::instance().Warning("Sound.Emulation.SampleFrequency (%d) is too low, using 11025 instead", sid_sample_frequency);
			sid_sample_frequency = 11025;
		}
		else if (sid_sample_frequency > 192000)
		{
			Logging::instance().Warning("Sound.Emulation.SampleFrequency (%d) is too high, using 192000 instead", sid_sample_frequency);
			sid_sample_frequency = 192000;
		}
		Logging::instance().Info("Sound.Emulation.SampleFrequency set to %d", sid_sample_frequency);

		sid_configuration.m_eSampleMethod = sid_use_resample ? SID_SAMPLE_METHOD_RESAMPLE_INTERPOLATE : SID_SAMPLE_METHOD_INTERPOLATE;
		sid_configuration.m_eModel = SID_MODEL_6581;
		sid_configuration.m_nSampleFrequency = sid_sample_frequency;

		m_SIDProxy = new SIDProxy(sid_configuration);
		m_CPUMemory = new CPUMemory(0x10000, &platform);
		m_CPU = new CPUmos6510();
		m_FlightRecorder = new FlightRecorder(&platform, 0x800);
		m_ExecutionHandler = new ExecutionHandler(m_CPU, m_CPUMemory, m_SIDProxy, m_FlightRecorder);

		// Create audio stream
		const int audio_buffer_size = GetSingleConfigurationValue<ConfigValueInt>(config, "Sound.Buffer.Size", 256);
		m_AudioStream = new AudioStream(sid_sample_frequency, 16, std::max<const int>(audio_buffer_size, 0x80), m_ExecutionHandler);

		// Create the main text field
		m_TextField = m_Viewport->CreateTextField(m_Viewport->GetClientWidth() / TextField::font_width, m_Viewport->GetClientHeight() / TextField::font_height, 0, 0);
		m_TextField->SetEnable(true);

		// Allocate empty Driver Info class
		m_DriverInfo = std::make_shared<DriverInfo>();

		// Create overlay control
		m_OverlayControl = std::make_unique<OverlayControl>(inViewport);

		// Create screens
		m_IntroScreen = std::make_unique<ScreenIntro>(
			m_Viewport,
			m_TextField,
			&m_CursorControl,
			m_DisplayState,
			m_KeyHookSetup.GetKeyHookStore(),
			m_DriverInfo,
			[&]() { OnExitIntroScreen(); },
			[&]() { OnExitIntroScreenToLoad(); });

		m_DiskScreen = std::make_unique<ScreenDisk>(
			&platform,
			m_Viewport,
			m_TextField,
			&m_CursorControl,
			m_DisplayState,
			m_KeyHookSetup.GetKeyHookStore(),
			config,
			[&](const std::string& inFilenameSelection, FileType inSaveFileType) { OnFilenameSelection(m_DiskScreen.get(), inFilenameSelection, inSaveFileType); },
			[&]() { OnCancelScreen(m_DiskScreen.get()); });

		m_ConvertScreen = std::make_unique<ScreenConvert>(
			m_Viewport,
			m_TextField,
			&m_CursorControl,
			m_DisplayState,
			m_KeyHookSetup.GetKeyHookStore(),
			&platform,
			[&]() { SetCurrentScreen(m_EditScreen.get()); },
			[&](ScreenBase* inCallerScreen, const std::string& inPathAndFilename, std::shared_ptr<Utility::C64File> inConversionResult) { return OnConversionSuccess(inCallerScreen, inPathAndFilename, inConversionResult); });

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
			[&](unsigned short inDestinationAddress, unsigned char inFirstZeroPage) { OnPack(m_EditScreen.get(), inDestinationAddress, inFirstZeroPage); },
			[&]() { m_FlipOverlayState = true; },
			[&](unsigned int inReconfigureOption) { Reconfigure(inReconfigureOption); });

		//
		// Apply additional configuration to the edit screen
		m_EditScreen->SetAdditionalConfiguration(
			GetSingleConfigurationValue<ConfigValueInt>(config, "Editor.Driver.ConvertLegacyColors", 0) != 0);
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

		IPlatform& platform = Global::instance().GetPlatform();
		ConfigFile& configFile = Global::instance().GetConfig();

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
			std::string default_driver_filename = GetSingleConfigurationValue<ConfigValueString>(configFile, "Editor.Driver.Default", std::string("sf2driver11_05.prg"));
			std::string drivers_folder = platform.Storage_GetDriversHomePath();
			LoadFile(drivers_folder + default_driver_filename);
		}

		// After loading, set the current path, so that opening the disk menu will be correct.
		const std::string default_start_path = platform.Storage_GetHomePath();
		std::string start_path = platform.OS_ParsePath(GetSingleConfigurationValue<ConfigValueString>(configFile, "Disk.Startup.Folder", default_start_path));

		if (!is_directory(start_path))
		{
			Logging::instance().Warning("%s is not a folder. Using default path.", start_path.c_str());
			start_path = default_start_path;
		}

		std::error_code ec;
		fs::current_path(start_path, ec);
		if (ec)
		{
			Logging::instance().Warning("Cannot change folder to %s.", start_path.c_str());

			// if this was a custom path, try again with the default
			if (start_path != default_start_path)
			{
				fs::current_path(default_start_path, ec);
				if (ec)
				{
					Logging::instance().Warning("Cannot change folder to %s.", default_start_path.c_str());
				}
			}
		}

		// Start the intro screen
		const bool skip_intro = GetSingleConfigurationValue<ConfigValueInt>(configFile, "Editor.Skip.Intro", 0) != 0;
		if (!skip_intro)
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
			m_CurrentScreen->TryQuit([&](bool inQuit) {
				if (inQuit)
					m_IsDone = true;
			});
		}
	}

	void EditorFacility::TryLoad(const std::string inPathAndFilename)
	{
		if (m_CurrentScreen != nullptr)
		{
			m_CurrentScreen->TryLoad(inPathAndFilename, [&, path_and_filename = inPathAndFilename](bool inQuit) {
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
		ConfigFile& configFile = Global::instance().GetConfig();

		if (inReconfigureOption == 0)
		{
			configFile.Reload();
			m_KeyHookSetup.Reset();
			m_KeyHookSetup.ApplyConfigSettings(configFile);
			ConfigureColorsFromScheme(m_SelectedColorScheme, *m_Viewport);

			m_EditScreen->SetActivationMessage("Reloaded config!");
			ForceRequestScreen(m_EditScreen.get());
		}
		else if (inReconfigureOption == 1)
		{
			m_SelectedColorScheme++;
			if (m_SelectedColorScheme >= m_ColorSchemeCount)
				m_SelectedColorScheme = 0;

			std::string selected_color_scheme_name = ConfigureColorsFromScheme(m_SelectedColorScheme, *m_Viewport);

			if (m_CurrentScreen == m_EditScreen.get())
			{
				m_EditScreen->SetActivationMessage("Selected color scheme: " + selected_color_scheme_name);
				ForceRequestScreen(m_EditScreen.get());
			}
		}
		else if (inReconfigureOption == 2)
		{
			std::string selected_color_scheme_name = ConfigureColorsFromScheme(m_SelectedColorScheme, *m_Viewport);

			if (m_CurrentScreen == m_EditScreen.get())
			{
				m_EditScreen->SetActivationMessage("Reloaded color scheme: " + selected_color_scheme_name);
				ForceRequestScreen(m_EditScreen.get());
			}
		}
		else if (inReconfigureOption == 3)
		{
			ConfigureColorsFromScheme(m_SelectedColorScheme, *m_Viewport);

			m_EditScreen->SetActivationMessage("[Selected song: " + std::to_string(m_DriverInfo->GetAuxilaryDataCollection().GetSongs().GetSelectedSong()) + "]");
			ForceRequestScreen(m_EditScreen.get());
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

		if (m_CurrentScreen != inRequestedScreen)
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

				if (c64_file != nullptr)
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

				// Flush copy/paste
				CopyPaste::Instance().Flush();

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

					// Flush copy/paste
					CopyPaste::Instance().Flush();

					return true;
				}
			}

			delete[] static_cast<char*>(data);
		}

		return false;
	}


	bool EditorFacility::LoadAndConvertFile(const std::string& inPathAndFilename, ScreenBase* inCallerScreen, std::function<void()> inSuccesfullConversionAction)
	{
		const int max_file_size = 16 * 1024 * 1024; // 16MB

		// Read test music data to cpu memory
		void* data = nullptr;
		long data_size = 0;

		auto on_successfull_conversion = [this, inPathAndFilename, inCallerScreen, inSuccesfullConversionAction](std::shared_ptr<Utility::C64File> inC64File) {
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

					// Flush copy/paste
					CopyPaste::Instance().Flush();

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
			auto do_save = [&, inFileName](std::string inTitle, std::string inAuthor, std::string inCopyright) {
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
				const unsigned char song_count = m_DriverInfo->GetAuxilaryDataCollection().GetSongs().GetSongCount();

				Utility::PSIDFile psid_file(
					data,
					data_size + 2,
					0,
					driver_common.m_UpdateAddress - driver_common.m_InitAddress,
					static_cast<unsigned short>(song_count),
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

				// Flush copy/paste
				CopyPaste::Instance().Flush();

				// Notify overlay
				m_OverlayControl->OnChange(*m_DriverInfo);

				return true;
			}
		}

		inCallerScreen->GetComponentsManager().StartDialog(std::make_shared<DialogMessage>("Conversion failed", "The selected file couldn't be converted correctly.", DefaultDialogWidth, true, []() {}));

		return false;
	}


	void EditorFacility::OnPack(ScreenBase* inCallerScreen, unsigned short inDestinationAddress, unsigned char inFirstZeroPage)
	{
		const bool is_uppercase = m_DisplayState.IsHexUppercase();

		Packer packer(*m_CPUMemory, *m_DriverInfo, inDestinationAddress, inFirstZeroPage);
		m_PackedData = packer.GetResult();

		std::string packing_info;
		packing_info += "Range: 0x" + EditorUtils::ConvertToHexValue(static_cast<unsigned short>(m_PackedData->GetTopAddress()), is_uppercase) + " - 0x" + EditorUtils::ConvertToHexValue(static_cast<unsigned short>(m_PackedData->GetBottomAddress()), is_uppercase) + "\n";
		packing_info += "Size : 0x" + EditorUtils::ConvertToHexValue(static_cast<unsigned short>(m_PackedData->GetDataSize()), is_uppercase);

		inCallerScreen->GetComponentsManager().StartDialog(
			std::make_shared<DialogMessage>("Packing results", packing_info, 30, false, [&]() {
				m_DiskScreen->SetMode(ScreenDisk::Mode::SavePacked);
				SetCurrentScreen(m_DiskScreen.get());
			}));
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
		auto on_success = [this]() {
			RequestScreen(m_EditScreen.get());
		};

		auto do_load = [this, on_success, inSelectedFilename, inCallerScreen]() {
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
			if (SaveFile(save_path_and_filename.string()))
				RequestScreen(m_EditScreen.get());
			else
				OnSaveError(inCallerScreen);
		}
		else if (IsFileSF2(save_path_and_filename.string()))
		{
			auto do_save = [save_path_and_filename, inCallerScreen, this]() {
				if (SaveFile(save_path_and_filename.string()))
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
			const bool confirm_quick_save = GetSingleConfigurationValue<ConfigValueInt>(Global::instance().GetConfig(), "Editor.Confirm.QuickSave", 1) != 0;
			
			auto do_save = [save_path_and_filename, inCallerScreen, this]() {
				if (SaveFile(save_path_and_filename.string()))
					this->m_EditScreen->SetStatusBarMessage(" Quick saved to: " + save_path_and_filename.filename().string(), 5000);
				else
					this->OnSaveError(inCallerScreen);
			};

			if (confirm_quick_save) {
				inCallerScreen->GetComponentsManager().StartDialog(std::make_shared<DialogMessageYesNo>("Warning", "Do you want to perform a quick save to:\n" + save_path_and_filename.string() + "?", DefaultDialogWidth, do_save, []() {}));
			}
			else {
				do_save();
			}
		
		}

	}


	void EditorFacility::DoImport(ScreenBase* inCallerScreen, const std::string& inSelectedFilename)
	{
		auto do_load = [&, inSelectedFilename, inCallerScreen]() {
			std::shared_ptr<DriverInfo> import_driver_info = nullptr;
			std::shared_ptr<Utility::C64File> import_c64_file = nullptr;

			if (LoadFileForImport(inSelectedFilename, import_driver_info, import_c64_file))
			{
				const bool valid_tables = import_driver_info->HasParsedHeaderBlock(DriverInfo::HeaderBlockID::ID_DriverTables);
				const bool valid_music_data = import_driver_info->HasParsedHeaderBlock(DriverInfo::HeaderBlockID::ID_MusicData);

				if (!valid_music_data)
					inCallerScreen->GetComponentsManager().StartDialog(std::make_shared<DialogMessage>("Import failure", "Unable to import music data!", DefaultDialogWidth, true, []() {}));
				else if (!valid_tables)
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
			auto do_save = [save_path_and_filename, this]() {
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
			auto do_save = [inCallerScreen, save_path_and_filename, this]() {
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


	std::string EditorFacility::ConfigureColorsFromScheme(int inSchemeIndex, Foundation::Viewport& inViewport)
	{
		ConfigFile& configFile = Global::instance().GetConfig();
		if (inSchemeIndex < m_ColorSchemeCount)
		{

			IPlatform& platform = Global::instance().GetPlatform();
			auto color_scheme_names = GetConfigurationValues<ConfigValueString>(configFile, "ColorScheme.Name", {});
			auto color_scheme_filenames = GetConfigurationValues<ConfigValueString>(configFile, "ColorScheme.Filename", {});

			std::string color_config_path_and_filename = platform.Storage_GetColorSchemesHomePath() + color_scheme_filenames[inSchemeIndex];
			ConfigFile color_config(platform, color_config_path_and_filename, configFile.GetValidSectionTags());

			if (color_config.IsValid())
			{
				Utility::Config::ConfigureColors(color_config, inViewport);
				return color_scheme_names[inSchemeIndex];
			}
		}

		Utility::Config::ConfigureColors(configFile, inViewport);
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
