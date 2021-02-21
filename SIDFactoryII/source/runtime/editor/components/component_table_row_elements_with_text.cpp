#include "runtime/editor/components/component_table_row_elements_with_text.h"
#include "runtime/editor/components/utils/text_editing_data_source_table_text.h"
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
		, m_TextWidth(inTextWidth)
	{
		m_TextEditingDataSourceTableText = std::make_shared<TextEditingDataSourceTableText>(inDataSourceTableText, inTextWidth);
	}

	ComponentTableRowElementsWithText::~ComponentTableRowElementsWithText()
	{
	}


	bool ComponentTableRowElementsWithText::HasDataChange() const
	{
		if (m_TextEditingDataSourceTableText->HasDataChange())
			return true;

		return ComponentTableRowElements::HasDataChange();
	}


	void ComponentTableRowElementsWithText::HandleDataChange()
	{
		if (m_TextEditingDataSourceTableText->HasDataChange()) 
			m_TextEditingDataSourceTableText->ApplyDataChange();

		ComponentTableRowElements::HandleDataChange();
	}


	bool ComponentTableRowElementsWithText::IsNoteInputSilenced() const
	{
		return IsEditingText();
	}


	void ComponentTableRowElementsWithText::ClearHasControl(CursorControl& inCursorControl)
	{
		if(IsEditingText())
			DoStopEditText(false);

		m_RequireRefresh = true;

		ComponentTableRowElements::ClearHasControl(inCursorControl);
	}



	bool ComponentTableRowElementsWithText::ConsumeInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		bool consume = false;

		if (IsEditingText())
			consume = m_TextEditingDataSourceTableText->ConsumeKeyboardInput(inKeyboard, inCursorControl);
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

		if (m_TextEditingDataSourceTableText->RequireRefresh())
		{
			m_RequireRefresh = true;
			m_TextEditingDataSourceTableText->ResetRequireRefresh();
		}

		return consume;
	}


	bool ComponentTableRowElementsWithText::ConsumeInput(const Foundation::Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		int top_row = m_TopRow;

		const bool consumed = ComponentTableRowElements::ConsumeInput(inMouse, inModifierKeyMask, inCursorControl, inComponentsManager);
		
		if (consumed)
		{
			Foundation::Point local_cell_position = GetLocalCellPosition(inMouse.GetPosition());

			const int text_x = m_Dimensions.m_Width + 1;
			const int text_x_right = text_x + m_TextWidth;

			if (local_cell_position.m_X >= text_x && local_cell_position.m_X <= text_x_right)
			{
				if (IsEditingText() && m_CursorY != m_TextEditingDataSourceTableText->GetTextLineIndex())
					DoStopEditText(false);
				if (!IsEditingText())
					DoStartEditText();

				const int cursor_pos = local_cell_position.m_X - text_x;

				m_TextEditingDataSourceTableText->TrySetCursorPosition(cursor_pos);
			}
			else
			{
				if (IsEditingText())
					DoStopEditText(false);
			}

			return true;
		}
		else
		{
			if (IsEditingText())
			{
				if (m_CursorY != m_TextEditingDataSourceTableText->GetTextLineIndex())
				{
					DoStopEditText(false);
					DoStartEditText();
				}
				else if (top_row != m_TopRow)
					m_TextEditingDataSourceTableText->UpdateScreenPosition(GetEditingTextScreenPosition(), inCursorControl);
			}
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

			Color color_highlight = ToColor(IsEditingText() ? UserColor::TableTextEditing : UserColor::TableText);

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


	bool ComponentTableRowElementsWithText::IsEditingText() const
	{
		return m_TextEditingDataSourceTableText->IsEditing();
	}

	
	void ComponentTableRowElementsWithText::DoStartEditText()
	{
		auto GetEditingRowIndex = [&]() -> unsigned int
		{
			return static_cast<unsigned int>(m_CursorY);
		};

		if (!IsEditingText())
			m_TextEditingDataSourceTableText->StartEditing(GetEditingRowIndex(), GetEditingTextScreenPosition());
	}


	void ComponentTableRowElementsWithText::DoStopEditText(bool inCancel)
	{
		m_TextEditingDataSourceTableText->StopEditing(inCancel);
	}


	Foundation::Point ComponentTableRowElementsWithText::GetEditingTextScreenPosition()
	{
		return Foundation::Point(m_Position.m_X + m_Rect.m_Dimensions.m_Width + 1, m_Position.m_Y + m_CursorY - m_TopRow);
	}
}
