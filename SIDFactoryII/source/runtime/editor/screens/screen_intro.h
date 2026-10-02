#pragma once

#include "screen_base.h"
#include "foundation/graphics/image.h"
#include "SDL.h"

#include <functional>
#include <memory>
#include <string>

class RtMidiOut;

namespace Emulation
{
	class USBSid;
}

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
			Emulation::USBSid* inUSBSID,
			std::shared_ptr<DriverInfo>& inDriverInfo,
			std::function<void(void)> inExitScreenCallback,
			std::function<void(void)> inExitScreenToLoadCallback
		);

		void Activate() override;
		void Deactivate() override;

		void TryQuit(std::function<void(bool)> inResponseCallback) override;
		void TryLoad(const std::string& inPathAndFilename, std::function<void(bool)> inResponseCallback) override;

		bool ConsumeKeyEvent(SDL_Keycode inKeyEvent, unsigned int inModifiers) override;
		void Update(int inDeltaTick) override;

	private:
		bool TryStartDialogForMidiOutDeviceSelection();
		bool TryStartDialogForUSBSIDBoardSelection();
		bool TryStartDialogForUSBSIDSIDSelection();

		void PrintCenteredText(int inY, const std::string& inText);
		Foundation::Image* CreateImageFromPNGData(const void* inData, int inDataSize);

		bool m_AddMidiPortSelectionOption;
		bool m_AddUSBSIDBoardSelectionOption;
		bool m_AddUSBSIDSIDSelectionOption;
		bool m_StartUSBSIDBoardSelection;

		std::function<void(void)> m_ExitScreenCallback;
		std::function<void(void)> m_ExitScreenToLoadCallback;
		std::shared_ptr<DriverInfo>& m_DriverInfo;

		Foundation::Image* m_Logo;
		RtMidiOut* m_RtMidiOut;
		Emulation::USBSid* m_USBSID;
	};
}
