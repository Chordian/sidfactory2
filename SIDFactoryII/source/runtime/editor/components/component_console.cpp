#include "component_console.h"

#include "foundation/graphics/textfield.h"
#include "foundation/input/mouse.h"
#include "foundation/input/keyboard.h"
#include "foundation/input/keyboard_utils.h"
#include "foundation/graphics/wrapped_string.h"
#include "runtime/editor/cursor_control.h"
#include "utils/usercolors.h"

#include "SDL_keycode.h"
#include "foundation/base/assert.h"

using namespace Foundation;
using namespace Utility;

namespace Editor
{
	ComponentConsole::ComponentConsole(int inID, int inGroupID, Undo* inUndo, Foundation::TextField* inTextField, int inX, int inY, int inWidth, int inHeight)
		: ComponentBase(inID, inGroupID, inUndo, inTextField, inX, inY, inWidth, inHeight)
		, m_TextColor(ToColor(UserColor::ConsoleText))
		, m_BackgroundColor(ToColor(UserColor::ConsoleBackground))
		, m_TextLines({""})
	{
		FOUNDATION_ASSERT(inTextField != nullptr);
	}

	ComponentConsole::~ComponentConsole()
	{

	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	ComponentConsole& ComponentConsole::operator << (const char* inString)
	{
		std::string string(inString);
		return this->operator << (string);
	}

	ComponentConsole& ComponentConsole::operator << (std::string& inText)
	{
		std::string new_line = m_TextLines.back() + inText;
		m_TextLines.pop_back();

		const size_t line_length = new_line.size();
		const size_t max_line_length = static_cast<size_t>(m_Dimensions.m_Width);

		size_t from = 0;

		for (size_t i=0; i<new_line.size(); ++i)
		{
			if (new_line[i] == '\n')
			{
				m_TextLines.push_back(new_line.substr(from, i - from));
				from = i + 1;
			}
			else if (i - from >= max_line_length)
			{
				m_TextLines.push_back(new_line.substr(from, i - from));
				from = i;
			}
		}

		m_TextLines.push_back(new_line.substr(from, line_length - from));

		m_RequireRefresh = true;
		return *this;
	}

	ComponentConsole& ComponentConsole::operator << (unsigned int inValue)
	{
		std::string string = std::to_string(inValue);
		return this->operator << (string);
	}


	ComponentConsole& ComponentConsole::operator << (int inValue)
	{
		std::string string = std::to_string(inValue);
		return this->operator << (string);
	}


	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentConsole::SetColors(const Foundation::Color& inTextColor, const Foundation::Color& inBackgroundColor)
	{
		m_TextColor = inTextColor;
		m_BackgroundColor = inBackgroundColor;
	}


	void ComponentConsole::SetHasControl(GetControlType inGetControlType, CursorControl& inCursorControl)
	{
		m_HasControl = true;
		inCursorControl.SetEnabled(false);
		m_RequireRefresh = true;
	}


	bool ComponentConsole::ConsumeInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
//		if (m_HasControl)
//		{
//			// Get key events
//			for (const auto& key_event : inKeyboard.GetKeyEventList())
//			{
//				switch (key_event)
//				{
//				default:
//					break;
//				}
//			}
//		}
//
		return false;
	}


	bool ComponentConsole::ConsumeInput(const Foundation::Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		if (m_MouseOver)
		{
		}

		return false;
	}


	bool ComponentConsole::ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse)
	{
		Foundation::Point cell_position = m_TextField->GetCellPositionFromPixelPosition(inMouse.GetPosition());

		const bool mouse_over = (cell_position.m_Y == m_Position.m_Y && cell_position.m_X >= m_Position.m_X && cell_position.m_X < m_Position.m_X + m_Dimensions.m_Width);
		m_MouseOver = mouse_over;

		return false;
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentConsole::Refresh(const DisplayState& inDisplayState)
	{
		if (m_RequireRefresh && m_TextField->IsEnabled())
		{
			Color color =  m_BackgroundColor;
			m_TextField->ColorAreaBackground(color, m_Rect);

			size_t display_line_count = static_cast<size_t>(GetDimensions().m_Height);
			size_t top_entry = m_TextLines.size() < display_line_count ? 0 : (m_TextLines.size() - display_line_count);

			for(size_t i=top_entry; i<m_TextLines.size(); ++i)
				m_TextField->Print(m_Position.m_X, m_Position.m_Y + i, m_TextColor, m_TextLines[i]);

			m_RequireRefresh = false;
		}
	}


	void ComponentConsole::HandleDataChange()
	{
		if (m_HasDataChange)
			m_HasDataChange = false;
	}


	void ComponentConsole::PullDataFromSource(const bool inFromUndo)
	{
	}


	void ComponentConsole::ExecuteInsertDeleteRule(const DriverInfo::TableInsertDeleteRule& inRule, int inSourceTableID, int inIndexPre, int inIndexPost)
	{

	}

	void ComponentConsole::ExecuteAction(int inActionInput)
	{

	}
}