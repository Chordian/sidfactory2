#include "screen_intro.h"

#include "foundation/graphics/viewport.h"
#include "foundation/graphics/image.h"
#include "foundation/graphics/textfield.h"
#include "foundation/graphics/wrapped_string.h"
#include "resources/data_logo.h"
#include "utils/utilities.h"
#include "libraries/picopng/picopng.h"
#include "runtime/editor/driver/driver_info.h"

#include <string>
#include <assert.h>



namespace Editor
{
	ScreenIntro::ScreenIntro(
		Foundation::Viewport* inViewport, 
		Foundation::TextField* inMainTextField,
		CursorControl* inCursorControl, 
		DisplayState& inDisplayState,
		Utility::KeyHookStore& inKeyHookStore,
		std::shared_ptr<DriverInfo>& inDriverInfo,
		std::function<void(void)> inExitScreenCallback,
		std::function<void(void)> inExitScreenToLoadCallback
	)
		: ScreenBase(inViewport, inMainTextField, inCursorControl, inDisplayState, inKeyHookStore)
		, m_DriverInfo(inDriverInfo)
		, m_ExitScreenCallback(inExitScreenCallback)
		, m_ExitScreenToLoadCallback(inExitScreenToLoadCallback)
	{
	}


	void ScreenIntro::Activate()
	{
		assert(m_DriverInfo != nullptr);
		ScreenBase::Activate();

		// Build string
		std::string build_string = "Development build: " + std::string(__DATE__) + " ";

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

		const int credits_margin = 25;
		const int credits_y = 26;
		const int driver_info_y = 41;
		const int continue_info_y = 43;
		const int build_y = dimensions.m_Height - 1;
		const int build_x = dimensions.m_Width;
        const int block_width = dimensions.m_Width >> 1;

		const Foundation::Rect credits_rect_left({ { credits_margin, credits_y }, { block_width - credits_margin, driver_info_y - credits_y - 1 } });
        const Foundation::Rect credits_rect_right({ { block_width, credits_y }, { block_width - credits_margin, driver_info_y - credits_y - 1 } });
        
		const Foundation::WrappedString credits_text_left("Editor facilities designed and suggested by:\n \nThomas Egeskov Petersen\nJens-Christian Huus\nTorben Korgaard Hansen\nThomas Laurits Mogensen\nThomas Bendt\nMichel de Bree\n \nProgramming by:\n \nThomas Egeskov Petersen", block_width);
        const Foundation::WrappedString credits_text_right("macOS version by:\nMichel de Bree\n \nreSID-fp Engine by:\nDag Lem\nand Antti S. Lankila \n \npicoPNG by:\nLode Vandevenne\n \nghc::filesystem for c++11 by:\nSteffen Schumann", block_width);

		m_MainTextField->PrintAligned(credits_rect_left, credits_text_left, Foundation::TextField::HorizontalAlignment::Center);
        m_MainTextField->PrintAligned(credits_rect_right, credits_text_right, Foundation::TextField::HorizontalAlignment::Center);

		if (m_DriverInfo->IsValid())
		{
			const std::string& driver_name = m_DriverInfo->GetDescriptor().m_DriverName;
			PrintCenteredText(driver_info_y, "Driver loaded: " + driver_name);
		}
		else
			PrintCenteredText(driver_info_y, "Driver has not been loaded!");

		PrintCenteredText(continue_info_y, "Press SPACE to continue, or F10 for disk menu!");

		m_MainTextField->Print(build_x - static_cast<int>(build_string.length()), build_y, Foundation::Color::Grey, build_string);
	}


	void ScreenIntro::Deactivate()
	{
		m_Viewport->Destroy(m_Logo);
	}


	void ScreenIntro::TryQuit(std::function<void(bool)> inResponseCallback)
	{
		if(inResponseCallback)
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
		else if (inKeyEvent == SDLK_F10)
		{
			m_ExitScreenToLoadCallback();
			return true;
		}

		return false;
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

