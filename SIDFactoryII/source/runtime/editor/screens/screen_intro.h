#pragma once

#include "screen_base.h"
#include "foundation/graphics/image.h"
#include "SDL.h"

#include <functional>
#include <memory>
#include <string>

class RtMidiOut;

namespace Editor
{
	class DriverInfo;
	class ScreenIntro final : public ScreenBase
	{
	public:
		ScreenIntro(
			Foundation::Viewport* inViewport, 
			Foundation::TextField* inMainTextField, 
			CursorControl* inCursorControl,
			DisplayState& inDisplayState,
			Utility::KeyHookStore& inKeyHookStore,
			RtMidiOut* inRtMidiOut,
			std::shared_ptr<DriverInfo>& inDriverInfo,
			std::function<void(void)> inExitScreenCallback,
			std::function<void(void)> inExitScreenToLoadCallback
		);

		void Activate() override;
		void Deactivate() override;

		void TryQuit(std::function<void(bool)> inResponseCallback) override;
		void TryLoad(const std::string& inPathAndFilename, std::function<void(bool)> inResponseCallback) override;

		bool ConsumeKeyEvent(SDL_Keycode inKeyEvent, unsigned int inModifiers) override;

	private:
		bool TryStartDialogForMidiOutDeviceSelection();
		
		void PrintCenteredText(int inY, const std::string& inText);
		Foundation::Image* CreateImageFromPNGData(const void* inData, int inDataSize);

		bool m_AddMidiPortSelectionOption;
		
		std::function<void(void)> m_ExitScreenCallback;
		std::function<void(void)> m_ExitScreenToLoadCallback;
		std::shared_ptr<DriverInfo>& m_DriverInfo;

		Foundation::Image* m_Logo;
		RtMidiOut* m_RtMidiOut;
	};
}
