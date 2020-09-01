#include "dialog_base.h"

#include "runtime/editor/components_manager.h"
#include "runtime/emulation/cpumemory.h"

#include <assert.h>

namespace Editor
{
	DialogBase::DialogBase()
		: m_Active(false)
		, m_Done(false)
	{

	}


	DialogBase::~DialogBase()
	{

	}


	void DialogBase::Cancel()
	{

	}


	void DialogBase::Activate(Foundation::Viewport* inViewport, CursorControl* inCursorControl)
	{
		m_CursorControl = inCursorControl;
		ActivateInternal(inViewport);
	}


	void DialogBase::Deactivate(Foundation::Viewport* inViewport)
	{
		DeactivateInternal(inViewport);
	}


	bool DialogBase::ConsumeInput(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse)
	{
		if (m_ComponentsManager != nullptr)
			return m_ComponentsManager->ConsumeInput(inKeyboard, inMouse);

		return false;
	}

	void DialogBase::Update(int inDeltaTick, Emulation::CPUMemory* inCPUMemory)
	{
		if (m_ComponentsManager != nullptr)
			m_ComponentsManager->Update(inDeltaTick, inCPUMemory);
	}

	void DialogBase::Refresh(const DisplayState& inDisplayState)
	{
		if (m_ComponentsManager != nullptr)
			m_ComponentsManager->Refresh(inDisplayState);
	}

	bool DialogBase::IsDone() const
	{
		return m_Done;
	}
}