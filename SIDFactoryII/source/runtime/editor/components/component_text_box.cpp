#include "component_text_box.h"

#include "foundation/graphics/textfield.h"
#include "foundation/input/mouse.h"
#include "foundation/input/keyboard.h"
#include "foundation/input/keyboard_utils.h"
#include "runtime/editor/cursor_control.h"
#include "utils/usercolors.h"

#include "SDL_keycode.h"
#include <assert.h>

using namespace Foundation;
using namespace Utility;

namespace Editor
{
	ComponentTextBox::ComponentTextBox(int inID, int inGroupID, Undo* inUndo, Foundation::TextField* inTextField, int inX, int inY, int inWidth, int inHeight)
		: ComponentBase(inID, inGroupID, inUndo, inTextField, inX, inY, inWidth, inHeight)
		, m_TextColor(ToColor(UserColor::ConsoleText))
		, m_BackgroundColor(ToColor(UserColor::ConsoleBackground))
	{
		assert(inTextField != nullptr);
	}

	ComponentTextBox::~ComponentTextBox()
	{

	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	ComponentTextBox& ComponentTextBox::operator << (const char* inString)
	{
		std::string string(inString);
		return this->operator << (string);
	}

	ComponentTextBox& ComponentTextBox::operator << (std::string& inText)
	{
		m_TextLines.push_back(inText);
		m_RequireRefresh = true;
		return *this;
	}


	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentTextBox::SetColors(const Foundation::Color& inTextColor, const Foundation::Color& inBackgroundColor)
	{
		m_TextColor = inTextColor;
		m_BackgroundColor = inBackgroundColor;
	}


	void ComponentTextBox::SetHasControl(GetControlType inGetControlType, CursorControl& inCursorControl)
	{
		m_HasControl = true;
		inCursorControl.SetEnabled(false);
		m_RequireRefresh = true;
	}


	bool ComponentTextBox::ConsumeInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		if (m_HasControl)
		{
			// Get key events
			for (const auto& key_event : inKeyboard.GetKeyEventList())
			{
				//switch (key_event)
				//{
				//default:
				//	break;
				//}
			}
		}

		return false;
	}


	bool ComponentTextBox::ConsumeInput(const Foundation::Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		if (m_MouseOver)
		{
		}

		return false;
	}


	void ComponentTextBox::ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse)
	{
		Foundation::Point cell_position = m_TextField->GetCellPositionFromPixelPosition(inMouse.GetPosition());

		const bool mouse_over = (cell_position.m_Y == m_Position.m_Y && cell_position.m_X >= m_Position.m_X && cell_position.m_X < m_Position.m_X + m_Dimensions.m_Width);
		m_MouseOver = mouse_over;
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentTextBox::Refresh(const DisplayState& inDisplayState)
	{
		if (m_RequireRefresh && m_TextField->IsEnabled())
		{
			Color color =  m_BackgroundColor;
			m_TextField->ColorAreaBackground(color, m_Rect);

			size_t display_line_count = static_cast<size_t>(GetDimensions().m_Height);
			size_t top_entry = m_TextLines.size() < display_line_count ? 0 : (display_line_count - m_TextLines.size());

			for(size_t i=top_entry; i<m_TextLines.size(); ++i)
				m_TextField->Print(m_Position.m_X, m_Position.m_Y + i, m_TextColor, m_TextLines[i]);
		}
	}


	void ComponentTextBox::HandleDataChange()
	{
		if (m_HasDataChange)
			m_HasDataChange = false;
	}


	void ComponentTextBox::PullDataFromSource()
	{

	}



	void ComponentTextBox::ExecuteInsertDeleteRule(const DriverInfo::TableInsertDeleteRule& inRule, int inSourceTableID, int inIndexPre, int inIndexPost)
	{

	}

	void ComponentTextBox::ExecuteAction(int inActionInput)
	{

	}
}