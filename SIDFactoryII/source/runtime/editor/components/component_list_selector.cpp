#include "component_list_selector.h"
#include "runtime/editor/cursor_control.h"
#include "foundation/input/mouse.h"
#include "foundation/input/keyboard.h"
#include "foundation/input/keyboard_utils.h"
#include "foundation/graphics/textfield.h"

using namespace Foundation;

namespace Editor
{
	ComponentListSelector::ComponentListSelector(int inID, int inGroupID, Undo* inUndo, std::shared_ptr<IDataSource> inDataSource, Foundation::TextField* inTextField, int inX, int inY, int inWidth, int inHeight, int inHorizontalMargin, int inVerticalMargin)
		: ComponentBase(inID, inGroupID, inUndo, inTextField, inX, inY, inWidth, inHeight)
		, m_DataSource(inDataSource)
		, m_HorizontalMargin(inHorizontalMargin)
		, m_VerticalMargin(inVerticalMargin)
		, m_BackgroundColor(Color::DarkBlue)
		, m_SelectionColor(Color::Red)
		, m_SelectionNoFocusColor(Color::DarkGrey)
		, m_TextColor(Color::White)
		, m_ContentX(inX + inHorizontalMargin)
		, m_ContentY(inY + inVerticalMargin)
		, m_ContentWidth(inWidth - (inHorizontalMargin << 1))
		, m_ContentHeight(inHeight - (inVerticalMargin << 1))
		, m_CursorPos(0)
		, m_TopVisibleIndex(0)
	{
	}

	ComponentListSelector::~ComponentListSelector()
	{

	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	bool ComponentListSelector::ConsumeInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		if (m_HasControl)
		{
			for (auto& key_event : inKeyboard.GetKeyEventList())
			{
				switch (key_event)
				{
				case SDLK_DOWN:
					if (inKeyboard.IsModifierEmpty())
					{
						DoCursorDown();
						m_RequireRefresh = true;
						return true;
					}
					break;
				case SDLK_UP:
					if (inKeyboard.IsModifierEmpty())
					{
						DoCursorUp();
						m_RequireRefresh = true;
						return true;
					}
					break;
				case SDLK_PAGEDOWN:
					DoPageDown();
					m_RequireRefresh = true;
					return true;
				case SDLK_PAGEUP:
					DoPageUp();
					m_RequireRefresh = true;
					return true;
				case SDLK_HOME:
					DoPageHome();
					m_RequireRefresh = true;
					return true;
				case SDLK_END:
					DoPageEnd();
					m_RequireRefresh = true;
					return true;
				}
			}
		}

		return false;
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	bool ComponentListSelector::ConsumeInput(const Foundation::Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		if (inMouse.IsButtonPressed(Mouse::Left))
		{
			Foundation::Point local_cell_position = m_TextField->GetCellPositionFromPixelPosition(inMouse.GetPosition()) - m_Position;

			if (local_cell_position.m_X >= m_VerticalMargin && local_cell_position.m_X < m_Dimensions.m_Width - m_VerticalMargin && local_cell_position.m_Y >= 0 && local_cell_position.m_Y < m_Dimensions.m_Height)
			{
				int cursor_pos = m_TopVisibleIndex + local_cell_position.m_Y - m_VerticalMargin;

				if (cursor_pos >= 0 && cursor_pos < m_DataSource->GetSize())
				{
					m_CursorPos = cursor_pos;
					m_RequireRefresh = true;
				}

				return true;
			}
		}

		DoMouseWheel(inMouse);

		return false;
	}


	void ComponentListSelector::ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse)
	{
		if (!m_HasControl)
			DoMouseWheel(inMouse);
	}


	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentListSelector::Refresh(const DisplayState& inDisplayState)
	{
		if (m_RequireRefresh)
		{
			m_TextField->ColorAreaBackground(m_BackgroundColor, m_Rect);
			m_TextField->ClearText(m_Rect);

			m_RequireRefresh = false;

			const int num_entries = m_ContentHeight;

			int index = m_TopVisibleIndex;
			int pos_y = m_ContentY;

			for (int i = 0; i < num_entries && index < m_DataSource->GetSize(); ++i)
			{
				RefreshLine(index, pos_y);

				if (index == m_CursorPos)
					m_TextField->ColorAreaBackground(m_HasControl ? m_SelectionColor : m_SelectionNoFocusColor, m_ContentX, pos_y, m_ContentWidth, 1);

				++index;
				++pos_y;
			}
		}
	}

	void ComponentListSelector::HandleDataChange()
	{

	}


	void ComponentListSelector::PullDataFromSource(const bool inFromUndo)
	{
	}


	void ComponentListSelector::ExecuteInsertDeleteRule(const DriverInfo::TableInsertDeleteRule& inRule, int inSourceTableID, int inIndexPre, int inIndexPost)
	{

	}


	void ComponentListSelector::ExecuteAction(int inActionInput)
	{

	}


	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentListSelector::SetHasControl(GetControlType inGetControlType, CursorControl& inCursorControl)
	{
		m_HasControl = true;

		// Handle disabling the cursor for the component
		if (m_TextField != nullptr)
			inCursorControl.SetEnabled(false);

		m_RequireRefresh = true;
	}

	void ComponentListSelector::ClearHasControl(CursorControl& inCursorControl)
	{
		m_HasControl = false;

		// Handle disabling the cursor for the component
		if (m_TextField != nullptr)
			inCursorControl.SetEnabled(false);

		m_RequireRefresh = true;
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentListSelector::SetColors(const Color& inBackground, const Color& inSelection, const Color& inSelectionNoFocus)
	{
		m_BackgroundColor = inBackground;
		m_SelectionColor = inSelection;
		m_SelectionNoFocusColor = inSelectionNoFocus;
	}


	void ComponentListSelector::SetColors(const Foundation::Color& inTextColor)
	{
		m_TextColor = inTextColor;
	}


	void ComponentListSelector::SetSelectionIndex(int inSelectionIndex)
	{
		m_CursorPos = inSelectionIndex;
		m_RequireRefresh = true;
	}

	const int ComponentListSelector::GetSelectionIndex() const
	{
		return m_CursorPos;
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentListSelector::DoMouseWheel(const Foundation::Mouse& inMouse)
	{
		Point scroll_wheel = inMouse.GetWheelDelta();

		if (scroll_wheel.m_Y != 0)
		{
			Point screen_position = inMouse.GetPosition();
			if (ContainsPosition(screen_position))
			{
				const int max_cursor_pos = m_DataSource->GetSize() - 1;
				const int visible_height = m_ContentHeight - 1;
				const int max_top_pos = max_cursor_pos < visible_height ? 0 : (max_cursor_pos - visible_height);

				int top_pos = m_TopVisibleIndex;
				int change = scroll_wheel.m_Y;

				while (change > 0)
				{
					--top_pos;
					--change;
				}
				while (change < 0)
				{
					++top_pos;
					++change;
				}

				top_pos = top_pos < 0 ? 0 : top_pos > max_top_pos ? max_top_pos : top_pos;

				if (top_pos != m_TopVisibleIndex)
				{
					m_TopVisibleIndex = top_pos;

					if (m_CursorPos < m_TopVisibleIndex)
						m_CursorPos = m_TopVisibleIndex;
					if (m_CursorPos > m_TopVisibleIndex + visible_height)
						m_CursorPos = m_TopVisibleIndex + visible_height;

					m_RequireRefresh = true;
				}
			}
		}
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentListSelector::RefreshLine(int inIndex, int inPosY)
	{
		m_TextField->Print(m_ContentX, inPosY, m_TextColor, "" + std::to_string(inIndex));
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentListSelector::DoCursorDown()
	{
		if (m_CursorPos < m_DataSource->GetSize() - 1)
		{
			++m_CursorPos;

			if (m_CursorPos >= m_TopVisibleIndex + m_ContentHeight)
				m_TopVisibleIndex = m_CursorPos - m_ContentHeight + 1;
		}
	}


	void ComponentListSelector::DoCursorUp()
	{
		if (m_CursorPos > 0)
		{
			--m_CursorPos;

			if (m_TopVisibleIndex > m_CursorPos)
				m_TopVisibleIndex = m_CursorPos;
		}
	}


	void ComponentListSelector::DoPageDown()
	{
		int new_cursor_pos = m_CursorPos + m_ContentHeight;
		
		if (new_cursor_pos >= m_DataSource->GetSize())
			new_cursor_pos = m_DataSource->GetSize() - 1;

		if (m_CursorPos != new_cursor_pos)
		{
			m_CursorPos = new_cursor_pos;

			if (m_CursorPos >= m_TopVisibleIndex + m_ContentHeight)
				m_TopVisibleIndex = m_CursorPos - m_ContentHeight + 1;
		}
	}


	void ComponentListSelector::DoPageUp()
	{
		int new_cursor_pos = m_CursorPos - m_ContentHeight;

		if (new_cursor_pos < 0)
			new_cursor_pos = 0;

		if (m_CursorPos != new_cursor_pos)
		{
			m_CursorPos = new_cursor_pos;

			if (m_TopVisibleIndex > m_CursorPos)
				m_TopVisibleIndex = m_CursorPos;
		}
	}


	void ComponentListSelector::DoPageHome()
	{
		if (m_CursorPos != 0)
		{
			m_CursorPos = 0;

			if (m_TopVisibleIndex > m_CursorPos)
				m_TopVisibleIndex = m_CursorPos;
		}
	}


	void ComponentListSelector::DoPageEnd()
	{
		const int new_cursor_pos = m_DataSource->GetSize() - 1;

		if (m_CursorPos != new_cursor_pos)
		{
			m_CursorPos = new_cursor_pos;

			if (m_CursorPos >= m_TopVisibleIndex + m_ContentHeight)
				m_TopVisibleIndex = m_CursorPos - m_ContentHeight + 1;
		}
	}



	std::string ComponentListSelector::CondenseString(const std::string& inString, const char* inPostFix, int inMaxLength) const
	{
		if (static_cast<int>(inString.length()) > inMaxLength)
		{
			std::string post_fix(inPostFix);
			std::string condensed_string = inString.substr(0, inMaxLength - post_fix.length());

			return condensed_string + post_fix;
		}

		return inString;
	}
}

