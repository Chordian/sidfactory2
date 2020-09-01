#pragma once

#include "runtime/editor/components_manager.h"
#include "runtime/editor/display_state.h"
#include "utils/keyhookstore.h"
#include <SDL.h>
#include <vector>
#include <functional>

namespace Foundation
{
	class Viewport;
	class TextField;
	class Keyboard;
	class Mouse;
}

namespace Utility
{
	template<typename CONTEXT>
	class KeyHook;
}

namespace Editor
{
	class CursorControl;
	class StatusBar;
	class ScreenBase
	{
	protected:
		ScreenBase(
			Foundation::Viewport* inViewport, 
			Foundation::TextField* inMainTextField, 
			CursorControl* inCursorControl, 
			DisplayState& inDisplayState,
			Utility::KeyHookStore& inKeyHookStore
		);
		virtual ~ScreenBase();

	public:
		virtual void Activate();
		virtual void Deactivate() = 0;

		virtual void TryQuit(std::function<void(bool)> inResponseCallback) = 0;
		virtual void TryLoad(const std::string& inPathAndFilename, std::function<void(bool)> inResponseCallback) = 0;

		virtual bool ConsumeInput(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse);

		virtual bool ConsumeKeyEvent(SDL_Keycode inKeyEvent, unsigned int inModifiers) = 0;
		virtual void Update(int inDeltaTick);
		virtual void Refresh();

		ComponentsManager& GetComponentsManager();

	protected:
		virtual void ConfigureKeys();
		void ClearTextField();

		void ShowQuitDialog(std::unique_ptr<ComponentsManager>& inComponentsManager, std::function<void(bool)> inResponseCallback);
		void ShowTryLoadDialog(const std::string& inPathAndFilename, std::unique_ptr<ComponentsManager>& inComponentsManager, std::function<void(bool)> inResponseCallback);

		std::unique_ptr<ComponentsManager> m_ComponentsManager;
		std::unique_ptr<StatusBar> m_StatusBar;

		CursorControl* m_CursorControl;
		DisplayState& m_DisplayState;

		std::vector<Utility::KeyHook<bool(void)>> m_KeyHooks;

		Foundation::Viewport* m_Viewport;
		Foundation::TextField* m_MainTextField;

		const Utility::KeyHookStore& m_KeyHookStore;
	};
}