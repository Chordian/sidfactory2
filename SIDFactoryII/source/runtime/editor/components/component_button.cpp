#include "component_button.h"

#include "foundation/graphics/textfield.h"
#include "foundation/input/mouse.h"
#include "foundation/input/keyboard.h"
#include "foundation/input/keyboard_utils.h"
#include "runtime/editor/cursor_control.h"
#include "utils/usercolors.h"

#include "SDL_keycode.h"
#include "foundation/base/assert.h"

using namespace Foundation;
using namespace Utility;

namespace Editor
{
	ComponentButton::ComponentButton(int inID, int inGroupID, Undo* inUndo, Foundation::TextField* inTextField, const std::string& inButtonText, int inX, int inY, int inWidth, std::function<void(void)> inButtonPressedCallback)
		: ComponentBase(inID, inGroupID, inUndo, inTextField, inX, inY, inWidth, 1)
		, m_ButtonText(inButtonText)
		, m_ButtonPressedCallback(inButtonPressedCallback)
		, m_MouseOver(false)
		, m_TextColor(ToColor(UserColor::ButtonText))
		, m_BackgroundColor(ToColor(UserColor::ButtonDefault))
	{
		FOUNDATION_ASSERT(inTextField != nullptr);
		FOUNDATION_ASSERT(inButtonText.length() <= static_cast<size_t>(inWidth));
	}

	ComponentButton::~ComponentButton()
	{

	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentButton::SetColors(const Foundation::Color& inTextColor, const Foundation::Color& inBackgroundColor)
	{
		m_TextColor = inTextColor;
		m_BackgroundColor = inBackgroundColor;
	}


	void ComponentButton::SetHasControl(GetControlType inGetControlType, CursorControl& inCursorControl)
	{
		m_HasControl = true;
		inCursorControl.SetEnabled(false);
		m_RequireRefresh = true;
	}


	bool ComponentButton::ConsumeInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		if (m_HasControl)
		{
			// Get key events
			for (const auto& key_event : inKeyboard.GetKeyEventList())
			{
				switch (key_event)
				{
				case SDLK_RETURN:
					m_ButtonPressedCallback();
					return true;
				default:
					break;
				}
			}
		}

		return false;
	}


	bool ComponentButton::ConsumeInput(const Foundation::Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		if(m_MouseOver)
		{
			if (inMouse.IsButtonPressed(Foundation::Mouse::Button::Left))
			{
				m_ButtonPressedCallback();
				return true;
			}
		}

		return false;
	}


	void ComponentButton::ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse)
	{
		Foundation::Point cell_position = m_TextField->GetCellPositionFromPixelPosition(inMouse.GetPosition());

		const bool mouse_over = (cell_position.m_Y == m_Position.m_Y && cell_position.m_X >= m_Position.m_X && cell_position.m_X < m_Position.m_X + m_Dimensions.m_Width);

		if (m_MouseOver != mouse_over)
			m_RequireRefresh = true;

		m_MouseOver = mouse_over;
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentButton::Refresh(const DisplayState& inDisplayState)
	{
		if (m_RequireRefresh && m_TextField->IsEnabled())
		{
			Color color = [&]()
			{
				if (m_MouseOver)
					return ToColor(m_HasControl ? UserColor::ButtonHighlightMouseOver : UserColor::ButtonDefaultMouseOver);
				else
					return m_HasControl ? ToColor(UserColor::ButtonHighlight) : m_BackgroundColor;
			}();
			m_TextField->ColorAreaBackground(color, m_Rect);

			int text_x = (m_Dimensions.m_Width - static_cast<int>(m_ButtonText.length())) >> 1;
			m_TextField->Print(m_Position.m_X + text_x, m_Position.m_Y, m_TextColor, m_ButtonText);
		}
	}


	void ComponentButton::HandleDataChange()
	{
		if (m_HasDataChange)
			m_HasDataChange = false;
	}


	void ComponentButton::PullDataFromSource(const bool inFromUndo)
	{
	}



	void ComponentButton::ExecuteInsertDeleteRule(const DriverInfo::TableInsertDeleteRule& inRule, int inSourceTableID, int inIndexPre, int inIndexPost)
	{

	}

	void ComponentButton::ExecuteAction(int inActionInput)
	{

	}
}