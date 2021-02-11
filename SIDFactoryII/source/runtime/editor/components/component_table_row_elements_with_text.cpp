#include "component_table_row_elements_with_text.h"
#include "runtime/editor/datasources/datasource_table.h"
#include "runtime/editor/datasources/datasource_table_text.h"
#include "foundation/graphics/textfield.h"
#include "foundation/input/keyboard.h"
#include "foundation/input/keyboard_utils.h"
#include "foundation/input/mouse.h"
#include "runtime/editor/cursor_control.h"
#include "utils/usercolors.h"



using namespace Foundation;
using namespace Utility;

namespace Editor
{
	ComponentTableRowElementsWithText::ComponentTableRowElementsWithText
	(
		int inID,
		int inGroupID,
		Undo* inUndo,
		std::shared_ptr<DataSourceTable> inDataSourceTable,
		std::shared_ptr<DataSourceTableText> inDataSourceTableText,
		Foundation::TextField* inTextField,
		int inX,
		int inY,
		int inHeight,
		int inTextWidth,
		bool inIndexAsContinuousMemory
	)
		: ComponentTableRowElements(inID, inGroupID, inUndo, inDataSourceTable, inTextField, inX, inY, inHeight, inIndexAsContinuousMemory)
		, m_DataSourceTableText(inDataSourceTableText)
		, m_HasDataChangeText(false)
		, m_EditingText(false)
		, m_TextWidth(inTextWidth)
		, m_TextEditCursorPos(0)
	{
	}

	ComponentTableRowElementsWithText::~ComponentTableRowElementsWithText()
	{
	}


	bool ComponentTableRowElementsWithText::HasDataChange() const
	{
		if (m_HasDataChangeText)
			return true;

		return ComponentTableRowElements::HasDataChange();
	}


	void ComponentTableRowElementsWithText::HandleDataChange()
	{
		if (m_HasDataChangeText)
		{
			m_DataSourceTableText->PushDataToSource();
			m_HasDataChangeText = false;
		}

		ComponentTableRowElements::HandleDataChange();
	}


	bool ComponentTableRowElementsWithText::IsNoteInputSilenced() const
	{
		return m_EditingText;
	}


	void ComponentTableRowElementsWithText::ClearHasControl(CursorControl& inCursorControl)
	{
		DoStopEditText(false);
		m_RequireRefresh = true;

		ComponentTableRowElements::ClearHasControl(inCursorControl);
	}



	bool ComponentTableRowElementsWithText::ConsumeInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		bool consume = false;

		if (m_EditingText)
		{
			for (auto& text_event : inKeyboard.GetKeyTextList())
			{
				if (KeyboardUtils::IsAcceptableInputText(text_event))
				{
					ApplyCharacter(text_event);
					consume = true;
				}
			}

			for (auto& key_event : inKeyboard.GetKeyEventList())
			{
				switch (key_event)
				{
				case SDLK_LEFT:
					DoCursorBackwards();
					consume = true;
					break;
				case SDLK_RIGHT:
					DoCursorForward();
					consume = true;
					break;
				case SDLK_INSERT:
					DoInsert();
					m_RequireRefresh = true;
					consume = true;
					break;
				case SDLK_DELETE:
					DoDelete();
					m_RequireRefresh = true;
					consume = true;
					break;
				case SDLK_BACKSPACE:
					DoBackspace(inKeyboard.IsModifierDownExclusive(Keyboard::Shift));
					m_RequireRefresh = true;
					consume = true;
					break;
				case SDLK_RETURN:
					DoStopEditText(false);
					m_RequireRefresh = true;
					consume = true;
					break;
				case SDLK_HOME:
					m_TextEditCursorPos = 0;
					m_RequireRefresh = true;
					consume = true;
					break;
				case SDLK_END:
					m_TextEditCursorPos = GetMaxPossibleCursorPosition();
					m_RequireRefresh = true;
					consume = true;
					break;
				case SDLK_ESCAPE:
					DoStopEditText(true);
					m_RequireRefresh = true;
					consume = true;
					break;
				}
			}

			ApplyCursorPosition(inCursorControl);
		}
		else
		{
			consume = ComponentTableRowElements::ConsumeInput(inKeyboard, inCursorControl, inComponentsManager);

			if (!consume)
			{
				for (auto key_code : inKeyboard.GetKeyEventList())
				{
					switch (key_code)
					{
					case SDLK_RETURN:
						if (inKeyboard.IsModifierEmpty())
						{
							DoStartEditText();
							m_RequireRefresh = true;
							consume = true;
						}
						break;
					}
				}
			}
		}

		return consume;
	}


	bool ComponentTableRowElementsWithText::ConsumeInput(const Foundation::Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		const bool consumed = ComponentTableRowElements::ConsumeInput(inMouse, inModifierKeyMask, inCursorControl, inComponentsManager);
		if (consumed)
		{
			Foundation::Point local_cell_position = GetLocalCellPosition(inMouse.GetPosition());

			const int text_x = m_Dimensions.m_Width + 1;
			const int text_x_right = text_x + m_TextWidth;

			if (local_cell_position.m_X >= text_x && local_cell_position.m_X <= text_x_right)
			{
				if (!m_EditingText)
					DoStartEditText();

				const int cursor_pos = local_cell_position.m_X - text_x;

				m_TextEditCursorPos = cursor_pos < GetMaxPossibleCursorPosition() ? cursor_pos : GetMaxPossibleCursorPosition();
				m_RequireRefresh = true;

				ApplyCursorPosition(inCursorControl);
			}
			else
			{
				if(m_EditingText)
					DoStopEditText(false);
			}

			return true;
		}

		return false;
	}


	void ComponentTableRowElementsWithText::ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse)
	{
		ComponentTableRowElements::ConsumeNonExclusiveInput(inMouse);
	}


	void ComponentTableRowElementsWithText::Refresh(const DisplayState& inDisplayState)
	{
		if (m_RequireRefresh)
		{
			const int text_x = m_Position.m_X + 1 + m_Dimensions.m_Width;
			
			int y = m_Position.m_Y;
			unsigned int row = m_TopRow;

			Color color_highlight = ToColor(m_EditingText ? UserColor::TableTextEditing : UserColor::TableText);

			m_TextField->ClearText(text_x, m_Position.m_Y, m_TextWidth, m_Dimensions.m_Height);
			m_TextField->ColorAreaBackground(ToColor(UserColor::TableTextBackground), text_x, m_Position.m_Y, m_TextWidth, m_Dimensions.m_Height);
			m_TextField->ColorArea(color_highlight, text_x, m_Position.m_Y, m_TextWidth, m_Dimensions.m_Height);

			for (int i = 0; i < m_Dimensions.m_Height; i++)
			{
				m_TextField->Print(text_x, y, row == static_cast<unsigned int>(m_CursorY) ? color_highlight : ToColor(UserColor::TableText), (*m_DataSourceTableText)[row], m_TextWidth);
				++row;
				++y;
			}
		}

		ComponentTableRowElements::Refresh(inDisplayState);
	}


	void ComponentTableRowElementsWithText::PullDataFromSource()
	{
		m_DataSourceTableText->PullDataFromSource();

		ComponentTableRowElements::PullDataFromSource();
	}



	bool ComponentTableRowElementsWithText::ContainsPosition(const Foundation::Point& inPixelPosition) const
	{
		Foundation::Rect extended_rect = m_Rect;

		extended_rect.m_Dimensions.m_Width += m_TextWidth + 1;
		return extended_rect.Contains(GetCellPosition(inPixelPosition));
	}


	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentTableRowElementsWithText::ApplyCharacter(char inCharacter)
	{
		std::string& text = (*m_DataSourceTableText)[m_CursorY];
		const int text_length = static_cast<int>(text.length());

		if (m_TextEditCursorPos < text_length)
			text[m_TextEditCursorPos] = inCharacter;
		else if (m_TextEditCursorPos == text_length && text_length < m_TextWidth)
			text += inCharacter;

		DoCursorForward();

		m_RequireRefresh = true;
		m_HasDataChangeText = true;
	}

	void ComponentTableRowElementsWithText::ApplyCursorPosition(CursorControl& inCursorControl)
	{
		int actual_cursor_x = m_Position.m_X + m_Rect.m_Dimensions.m_Width + 1 + m_TextEditCursorPos;
		int actual_cursor_y = m_Position.m_Y + m_CursorY - m_TopRow;

		inCursorControl.SetPosition(CursorControl::Position({ actual_cursor_x, actual_cursor_y }));
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentTableRowElementsWithText::DoStartEditText()
	{
		if (!m_EditingText)
		{
			m_EditingText = true;
			m_TextEditCursorPos = 0;
			m_PreEditTextValue = (*m_DataSourceTableText)[m_CursorY];
		}
	}

	void ComponentTableRowElementsWithText::DoStopEditText(bool inCancel)
	{
		if (m_EditingText)
		{
			if (inCancel)
				(*m_DataSourceTableText)[m_CursorY] = m_PreEditTextValue;

			m_EditingText = false;
		}
	}

	void ComponentTableRowElementsWithText::DoCursorForward()
	{
		if (m_TextEditCursorPos < GetMaxPossibleCursorPosition() )
			++m_TextEditCursorPos;
	}

	void ComponentTableRowElementsWithText::DoCursorBackwards()
	{
		if (m_TextEditCursorPos > 0)
			--m_TextEditCursorPos;
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentTableRowElementsWithText::DoInsert()
	{
		std::string& text = (*m_DataSourceTableText)[m_CursorY];
		const int text_length = static_cast<int>(text.length());

		const auto last_character = text[text_length - 1];

		for (int i = text_length - 2; i >= m_TextEditCursorPos; --i)
			text[i + 1] = text[i];

		text[m_TextEditCursorPos] = ' ';

		if (text_length < m_TextWidth)
			text += last_character;

		m_HasDataChangeText = true;
	}

	void ComponentTableRowElementsWithText::DoDelete()
	{
		std::string& text = (*m_DataSourceTableText)[m_CursorY];
		const int text_length = static_cast<int>(text.length());

		std::string new_text = text.substr(0, m_TextEditCursorPos);
		if (m_TextEditCursorPos < text_length)
			new_text += text.substr(m_TextEditCursorPos + 1, text_length - m_TextEditCursorPos);

		text = new_text;

		m_HasDataChangeText = true;
	}

	void ComponentTableRowElementsWithText::DoBackspace(bool inIsShiftDown)
	{
		if (inIsShiftDown)
			DoInsert();
		else if (m_TextEditCursorPos > 0)
		{
			DoCursorBackwards();
			DoDelete();
		}
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	const int ComponentTableRowElementsWithText::GetMaxPossibleCursorPosition() const
	{
		const int pos = static_cast<int>((*m_DataSourceTableText)[m_CursorY].length());
		return pos >= m_TextWidth ? m_TextWidth - 1 : pos;
	}

}
