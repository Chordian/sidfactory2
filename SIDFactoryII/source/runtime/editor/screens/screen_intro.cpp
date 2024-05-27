#include "screen_intro.h"

#include "foundation/graphics/image.h"
#include "foundation/graphics/textfield.h"
#include "foundation/graphics/viewport.h"
#include "foundation/graphics/wrapped_string.h"
#include "libraries/picopng/picopng.h"
#include "resources/data_logo.h"
#include "runtime/editor/driver/driver_info.h"
#include "utils/utilities.h"

#include "foundation/base/assert.h"
#include "libraries/rtmidi/RtMidi.h"
#include "runtime/editor/dialog/dialog_selection_list.h"
#include "utils/rtmidi_utils.h"

#include <string>

namespace Editor
{
	using namespace Utility;
	
	ScreenIntro::ScreenIntro(
		Foundation::Viewport* inViewport,
		Foundation::TextField* inMainTextField,
		CursorControl* inCursorControl,
		DisplayState& inDisplayState,
		KeyHookStore& inKeyHookStore,
		RtMidiOut* inRtMidiOut,
		std::shared_ptr<DriverInfo>& inDriverInfo,
		std::function<void(void)> inExitScreenCallback,
		std::function<void(void)> inExitScreenToLoadCallback)
		: ScreenBase(inViewport, inMainTextField, inCursorControl, inDisplayState, inKeyHookStore)
		, m_DriverInfo(inDriverInfo)
		, m_ExitScreenCallback(inExitScreenCallback)
		, m_ExitScreenToLoadCallback(inExitScreenToLoadCallback)
		, m_RtMidiOut(inRtMidiOut)
		, m_AddMidiPortSelectionOption(false)
	{
	}


	void ScreenIntro::Activate()
	{
		FOUNDATION_ASSERT(m_DriverInfo != nullptr);
		ScreenBase::Activate();

		m_AddMidiPortSelectionOption = !RtMidiUtils::RtMidiOut_HasOpenPort(m_RtMidiOut);
		
		// Build string
#ifdef _BUILD_NR
		const std::string build_number = _BUILD_NR;
#else
		const std::string build_number = std::string(__DATE__);
#endif

		const std::string build_string = "Build " + build_number + " ";

		// Load bmp
		void* file_buffer;
		long file_size;

		if (Utility::ReadFile("logo.png", 0, &file_buffer, file_size))
		{
			m_Logo = CreateImageFromPNGData(file_buffer, static_cast<int>(file_size));
			delete[] static_cast<char*>(file_buffer);
		}
		else
			m_Logo = m_Viewport->CreateImageFromFile("logo.bmp");

		if (m_Logo == nullptr)
			m_Logo = CreateImageFromPNGData(static_cast<const void*>(Resource::data_logo), sizeof(Resource::data_logo));

		if (m_Logo != nullptr)
		{
			Foundation::Extent logo_dimensions = m_Logo->GetDimensions();
			int logo_x = (m_Viewport->GetClientWidth() - logo_dimensions.m_Width) >> 1;

			m_Logo->SetPosition({ logo_x, 0 });
		}

		// Clear the text field
		ClearTextField();

		const auto& dimensions = m_MainTextField->GetDimensions();

		const int credits_y = 26;
		const int driver_info_y = 41;
		const int continue_info_y = 43;
		const int build_y = dimensions.m_Height - 1;
		const int build_x = dimensions.m_Width;
		const int num_blocks = 3;
		const int block_width = dimensions.m_Width / (num_blocks + 1);
		const int credits_margin = (dimensions.m_Width - (block_width * num_blocks)) >> 1;
		
		const auto OutputBlock = [&](const int inBlock, const std::string& InText)
		{
			const Foundation::Rect rect({ { credits_margin + (inBlock * block_width), credits_y }, { block_width, driver_info_y - credits_y - 1 } });
			m_MainTextField->PrintAligned(rect, Foundation::WrappedString(InText, block_width), Foundation::TextField::HorizontalAlignment::Center);
		};
		
		OutputBlock(0, "Programming by:\nThomas Egeskov Petersen\nJens-Christian Huus\nMichel de Bree\nThomas Jansson\n \nAdditional design and suggestions by:\n Torben Korgaard Hansen\nThomas Laurits Mogensen\nThomas Bendt");
		OutputBlock(1, "reSID-fp Engine by:\nDag Lem\nAntti S. Lankila \n \npicoPNG by:\nLode Vandevenne\n \nminiz by:\nRich Geldreich");
		OutputBlock(2, "ghc::filesystem for c++11 by:\nSteffen Schumann\n \nRtMidi by:\nGary P. Scavone");
		
		if (m_DriverInfo->IsValid())
		{
			const std::string& driver_name = m_DriverInfo->GetDescriptor().m_DriverName;
			PrintCenteredText(driver_info_y, "Driver loaded: " + driver_name);
		}
		else
			PrintCenteredText(driver_info_y, "Driver has not been loaded!");

		if(m_AddMidiPortSelectionOption)
			PrintCenteredText(continue_info_y, "Press SPACE to continue, F1 to choose midi output device or F10 for disk menu!");
		else
			PrintCenteredText(continue_info_y, "Press SPACE to continue or F10 for disk menu!");

		m_MainTextField->Print(build_x - static_cast<int>(build_string.length()), build_y, Foundation::Color::Grey, build_string);
	}


	void ScreenIntro::Deactivate()
	{
		m_Viewport->Destroy(m_Logo);
	}


	void ScreenIntro::TryQuit(std::function<void(bool)> inResponseCallback)
	{
		if (inResponseCallback)
			inResponseCallback(true);
	}


	void ScreenIntro::TryLoad(const std::string& inPathAndFilename, std::function<void(bool)> inResponseCallback)
	{
		if (inResponseCallback)
			inResponseCallback(true);
	}


	bool ScreenIntro::ConsumeKeyEvent(SDL_Keycode inKeyEvent, unsigned int inModifiers)
	{
		if (inKeyEvent == SDLK_SPACE)
		{
			m_ExitScreenCallback();
			return true;
		}
		if (inKeyEvent == SDLK_F1 && m_AddMidiPortSelectionOption)
		{
			// Invoke midi device selection screen and invoke midi device
			TryStartDialogForMidiOutDeviceSelection();
			
			return true;
		}
		if (inKeyEvent == SDLK_F10)
		{
			m_ExitScreenToLoadCallback();
			return true;
		}

		return false;
	}


	bool ScreenIntro::TryStartDialogForMidiOutDeviceSelection()
	{
		std::vector<std::string> selections;

		const auto MidiOutPorts = RtMidiUtils::RtMidiOut_GetPorts(m_RtMidiOut);
		if(MidiOutPorts.empty())
			return false;

		for (const auto& MidiOutPort : MidiOutPorts)
		{
			std::string selection_string = "Midi device: " + std::to_string(MidiOutPort.m_PortNumber) + (MidiOutPort.m_PortNumber < 10 ? "  [" : " [") + MidiOutPort.m_PortName + "]";
			selections.push_back(selection_string);
		}

		m_ComponentsManager->StartDialog(
			std::make_shared<DialogSelectionList>
			(
				60,
				MidiOutPorts.size() + 3,
				0,
				"Select midi output device!",
				selections,
				[this, MidiOutPorts](const unsigned int inSelectionIndex)
				{
					RtMidiUtils::RtMidiOut_OpenPort(m_RtMidiOut, MidiOutPorts[inSelectionIndex]);
					m_ExitScreenCallback();
				},
				[]() {}
			)
		);

		return true;
	}



	void ScreenIntro::PrintCenteredText(int inY, const std::string& inText)
	{
		const Foundation::Extent& dimensions = m_MainTextField->GetDimensions();
		const Foundation::WrappedString message = { inText, dimensions.m_Width };

		m_MainTextField->PrintAligned({ { 0, inY }, { dimensions.m_Width, 1 } }, message, Foundation::TextField::HorizontalAlignment::Center);
	}


	Foundation::Image* ScreenIntro::CreateImageFromPNGData(const void* inData, int inDataSize)
	{
		std::vector<unsigned char> decoded_image;
		unsigned long decoded_image_width;
		unsigned long decoded_image_height;

		if (PicoPNG::decodePNG(decoded_image, decoded_image_width, decoded_image_height, static_cast<const unsigned char*>(inData), inDataSize, true) == 0)
		{
			unsigned char* data = new unsigned char[decoded_image.size()];
			for (unsigned int i = 0; i < decoded_image.size(); ++i)
				data[i] = decoded_image[i];
			Foundation::Image* image = m_Viewport->CreateImageFromARGBData(static_cast<void*>(data), static_cast<unsigned int>(decoded_image_width), static_cast<unsigned int>(decoded_image_height), true);
			delete[] data;

			return image;
		}

		return nullptr;
	}
}
