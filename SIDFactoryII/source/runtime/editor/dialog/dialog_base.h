#pragma once

#include "runtime/editor/cursor_control.h"
#include <memory>

namespace Foundation
{
	class Viewport;
	class Keyboard;
	class Mouse;
}

namespace Emulation
{
	class CPUMemory;
}

namespace Editor
{
	class CursorControl;
	class ScreenBase;
	class ComponentsManager;
	class DisplayState;

	class DialogBase
	{
	protected:
		DialogBase();

	public:
		virtual ~DialogBase();

		void Activate(Foundation::Viewport* inViewport, CursorControl* inCursorControl);
		void Deactivate(Foundation::Viewport* inViewport);
		
		virtual void Cancel();

		virtual bool ConsumeInput(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse);
		virtual void Update(int inDeltaTick, Emulation::CPUMemory* inCPUMemory);
		virtual void Refresh(const DisplayState& inDisplayState);

		bool IsDone() const;

	protected:
		virtual void ActivateInternal(Foundation::Viewport* inViewport) = 0;
		virtual void DeactivateInternal(Foundation::Viewport* inViewport) = 0;

		bool m_Done;

		CursorControl* m_CursorControl;
		std::unique_ptr<ComponentsManager> m_ComponentsManager;

	private:
		bool m_Active;
	};
}