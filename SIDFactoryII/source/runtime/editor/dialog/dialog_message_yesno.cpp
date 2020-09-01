#include "dialog_message_yesno.h"

#include "foundation/input/keyboard.h"
#include "foundation/graphics/textfield.h"

#include "runtime/editor/components_manager.h"

namespace Editor
{
	DialogMessageYesNo::DialogMessageYesNo(const std::string& inCaption, const std::string& inMessage, int inWidth, std::function<void(void)>&& inYes, std::function<void(void)>&& inNo)
		: DialogMessage(inCaption, inMessage, inWidth, true)
		, m_YesFunction(inYes)
		, m_NoFunction(inNo)
	{

	}


	void DialogMessageYesNo::Cancel()
	{
		if (m_NoFunction)
			m_NoFunction();
	}


	bool DialogMessageYesNo::ConsumeInput(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse)
	{
		DialogBase::ConsumeInput(inKeyboard, inMouse);

		for (const auto& key_event : inKeyboard.GetKeyEventList())
		{
			switch (key_event)
			{
			case SDLK_ESCAPE:
				m_Done = true;
				m_NoFunction();
				return true;
			case SDLK_y:
				m_Done = true;
				m_YesFunction();
				return true;
			case SDLK_n:
				m_Done = true;
				m_NoFunction();
				return true;
			}
		}

		return false;
	}


	void DialogMessageYesNo::ActivateInternal(Foundation::Viewport* inViewport)
	{
		DialogMessage::ActivateInternal(inViewport);

		m_ComponentsManager = std::make_unique<ComponentsManager>(inViewport, m_CursorControl);
		m_ComponentsManager->SetGroupEnabledForInput(0, true);
		// m_ComponentsManager->SetGroupEnabledForTabbing(0);

		const int button_width = 12;
		const int button_pos_x = 2;

		m_ComponentYesButton = std::make_shared<ComponentButton>(1, 0, nullptr,
			m_TextField, "Yes", 
			button_pos_x, GetMessageBottomPositionY() + 1, 
			button_width, 
			[&]() { m_Done = true; m_YesFunction(); });
		m_ComponentNoButton = std::make_shared<ComponentButton>(2, 0, nullptr,
			m_TextField, "No",
			m_TextField->GetDimensions().m_Width - (button_pos_x + button_width), GetMessageBottomPositionY() + 1,
			button_width,
			[&]() { m_Done = true; m_NoFunction(); });

		m_ComponentsManager->AddComponent(m_ComponentNoButton);
		m_ComponentsManager->AddComponent(m_ComponentYesButton);
		m_ComponentsManager->SetComponentInFocus(0);
	}


	int DialogMessageYesNo::GetDialogHeight() const
	{
		return DialogMessage::GetDialogHeight() + 2;
	}
}