#include "screen_base.h"

#include "foundation/graphics/viewport.h"
#include "foundation/graphics/textfield.h"
#include "foundation/input/keyboard.h"
#include "runtime/editor/screens/statusbar/status_bar.h"
#include "runtime/editor/dialog/dialog_message_yesno.h"
#include "utils/keyhook.h"

#include <assert.h>
#include <memory>

using namespace Foundation;

namespace Editor
{
	ScreenBase::ScreenBase(
		Foundation::Viewport* inViewport,
		Foundation::TextField* inMainTextField, 
		CursorControl* inCursorControl, 
		DisplayState& inDisplayState,
		Utility::KeyHookStore& inKeyHookStore
	)
		: m_Viewport(inViewport)
		, m_MainTextField(inMainTextField)
		, m_CursorControl(inCursorControl)
		, m_DisplayState(inDisplayState)
		, m_KeyHookStore(inKeyHookStore)
	{
		m_ComponentsManager = std::make_unique<ComponentsManager>(inViewport, inCursorControl);

		ConfigureKeys();
	}

	ScreenBase::~ScreenBase()
	{

	}

	void ScreenBase::Activate()
	{
		ConfigureKeys();
	}

	//------------------------------------------------------------------------------------------------------------

	bool ScreenBase::ConsumeInput(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse)
	{
		const bool was_displaying_dialog = m_ComponentsManager->IsDisplayingDialog();

		if (m_ComponentsManager->ConsumeInput(inKeyboard, inMouse))
			return true;
		
		if (!m_ComponentsManager->IsDisplayingDialog() && !was_displaying_dialog)
		{
			const unsigned int modifier_mask = inKeyboard.GetModiferMask();

			for (const auto key_event : inKeyboard.GetKeyEventList())
			{
				if (Utility::ConsumeInputKeyHooks(key_event, modifier_mask, m_KeyHooks))
					return true;
				if (ConsumeKeyEvent(key_event, modifier_mask))
					return true;
			}
		}

		if (m_StatusBar != nullptr)
			m_StatusBar->ConsumeInput(inMouse, inKeyboard.GetModiferMask());

		return false;
	}


	void ScreenBase::Update(int inDeltaTick)
	{
		if (m_StatusBar != nullptr)
			m_StatusBar->Update(inDeltaTick);

		m_ComponentsManager->Update(inDeltaTick, nullptr);
	}


	void ScreenBase::Refresh()
	{
		if (m_StatusBar != nullptr)
			m_StatusBar->Refresh();
	}


	//------------------------------------------------------------------------------------------------------------

	ComponentsManager& ScreenBase::GetComponentsManager()
	{
		return *m_ComponentsManager;
	}

	//------------------------------------------------------------------------------------------------------------

	void ScreenBase::ConfigureKeys()
	{

	}

	void ScreenBase::ClearTextField()
	{
		assert(m_MainTextField != nullptr);

		// Clear the text from the top line (but keep coloring)
		m_MainTextField->Clear();

		// Clear the status bar
		if (m_StatusBar != nullptr)
			m_StatusBar->Clear();
	}


	void ScreenBase::ShowQuitDialog(std::unique_ptr<ComponentsManager>& inComponentsManager, std::function<void(bool)> inResponseCallback)
	{
		if (inComponentsManager->IsDisplayingDialog())
			inComponentsManager->CancelDialog();

		inComponentsManager->StartDialog(std::make_shared<DialogMessageYesNo>("Quit SID Factory II", "Are you sure you want exit the editor?\nAny unsaved changes will be lost!", 100, [inResponseCallback]() { inResponseCallback(true); }, []() {}));
	}


	void ScreenBase::ShowTryLoadDialog(const std::string& inPathAndFilename, std::unique_ptr<ComponentsManager>& inComponentsManager, std::function<void(bool)> inResponseCallback)
	{
		if (inComponentsManager->IsDisplayingDialog())
			inComponentsManager->CancelDialog();

		inComponentsManager->StartDialog(std::make_shared<DialogMessageYesNo>("Load file", "Do you want to load the file?\n" + inPathAndFilename + "\nAny unsaved changes will be lost!", 120, [inResponseCallback]() { inResponseCallback(true); }, []() {}));
	}
}
