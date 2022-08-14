#include "component_table_row_elements.h"

#include "foundation/graphics/textfield.h"
#include "foundation/input/mouse.h"
#include "foundation/input/keyboard.h"
#include "foundation/input/keyboard_utils.h"
#include "runtime/editor/cursor_control.h"
#include "runtime/editor/display_state.h"
#include "runtime/editor/undo/undo.h"
#include "runtime/editor/undo/undo_componentdata/undo_componentdata_table_row_elements.h"
#include "runtime/editor/dialog/dialog_bit_input.h"
#include "runtime/editor/components_manager.h"
#include "utils/usercolors.h"

#include "SDL_keycode.h"
#include "foundation/base/assert.h"

using namespace Foundation;

namespace Editor
{
	int ComponentTableRowElements::GetWidthFromColumns(int inColumnCount)
	{
		// Example, 3 rows: "xx: 01 02 03"
		return 3 * (inColumnCount + 1);
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	ComponentTableRowElements::ComponentTableRowElements(
		int inID,
		int inGroupID,
		Undo* inUndo,
		std::shared_ptr<DataSourceTable> inDataSource,
		Foundation::TextField* inTextField,
		int inX,
		int inY,
		int inHeight,
		bool inIndexAsContinuousMemory)
		: ComponentBase(inID, inGroupID, inUndo, inTextField, inX, inY, GetWidthFromColumns(inDataSource->GetColumnCount()), inHeight)
		, m_DataSource(inDataSource)
		, m_InsertDeleteEnabled(false)
		, m_TopRow(0)
		, m_CursorX(0)
		, m_CursorY(0)
		, m_MaxCursorX(inDataSource->GetColumnCount() * 2 - 1)
		, m_MaxCursorY(inDataSource->GetRowCount() - 1)
		, m_HasLocalDataChange(false)
		, m_SpaceBarPressed(false)
		, m_IndexAsContinuousMemory(inIndexAsContinuousMemory)
		, m_HighlightRowOfSelectedSongIndex(false)
	{
		FOUNDATION_ASSERT(inDataSource != nullptr);
		FOUNDATION_ASSERT(inTextField != nullptr);
	}

	ComponentTableRowElements::~ComponentTableRowElements()
	{

	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentTableRowElements::EnableInsertDelete(bool inEnabled)
	{
		m_InsertDeleteEnabled = inEnabled;
	}


	void ComponentTableRowElements::SetInsertDeleteCallback(std::function<void(int, int, int)> inInsertDeleteCallback)
	{
		m_InsertDeleteCallback = inInsertDeleteCallback;
	}


	void ComponentTableRowElements::SetTableActionRules(const DriverInfo::TableActionRules& inActionRules)
	{
		m_ActionRules = inActionRules;
	}


	void ComponentTableRowElements::SetActionRuleExecutionCallback(std::function<void(int, int)> inActionRuleCallback)
	{
		m_ActionRuleExecutionCallback = inActionRuleCallback;
	}


	void ComponentTableRowElements::SetSpaceBarPressedCallback(std::function<void(bool, bool)> inSpaceBarPressedCallback)
	{
		m_SpaceBarCallback = inSpaceBarPressedCallback;
	}


	const int ComponentTableRowElements::GetSelectedRow() const
	{
		return m_CursorY;
	}


	ComponentTableRowElements::SelectedRowChangedEvent& ComponentTableRowElements::GetSelectedRowChangedEvent()
	{
		return m_SelectedRowChangedEvent;
	}


	const DataSourceTable* ComponentTableRowElements::GetDataSource() const
	{
		return &(*m_DataSource);
	}


	void ComponentTableRowElements::DoCursorUp()
	{
		SetRow(m_CursorY > 0 ? (m_CursorY - 1) : 0);
		AdjustVisibleArea();
		m_RequireRefresh = true;
	}

	void ComponentTableRowElements::DoCursorDown()
	{
		SetRow(m_CursorY < m_MaxCursorY ? (m_CursorY + 1) : m_MaxCursorY);
		AdjustVisibleArea();
		m_RequireRefresh = true;
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentTableRowElements::ClearHasControl(CursorControl& inCursorControl)
	{
		if (m_SpaceBarPressed)
			OnSpaceBarPressed(false, false);

		ComponentBase::ClearHasControl(inCursorControl);
	}

	bool ComponentTableRowElements::ConsumeInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		bool consume = false;

		if (m_HasControl)
		{
			{
				// Get key events
				const std::vector<SDL_Keycode>& key_event_list = inKeyboard.GetKeyEventList();

				for (const auto& key_event : key_event_list)
				{
					switch (key_event)
					{
					case SDLK_LEFT:
						if (inKeyboard.IsModifierEmpty())
						{
							m_CursorX = m_CursorX > 0 ? (m_CursorX - 1) : 0;
							consume = true;
						}
						break;
					case SDLK_RIGHT:
						if (inKeyboard.IsModifierEmpty())
						{
							m_CursorX = m_CursorX < m_MaxCursorX ? (m_CursorX + 1) : m_MaxCursorX;
							consume = true;
						}
						break;
					case SDLK_UP:
						if (inKeyboard.IsModifierEmpty())
						{
							DoCursorUp();
							consume = true;
						}
						break;
					case SDLK_DOWN:
						if (inKeyboard.IsModifierEmpty())
						{
							DoCursorDown();
							consume = true;
						}
						break;
					case SDLK_INSERT:
						if (m_InsertDeleteEnabled)
						{
							DoInsert();
							m_RequireRefresh = true;
							consume = true;
						}
						break;
					case SDLK_DELETE:
						if (m_InsertDeleteEnabled)
						{
							DoDelete();
							m_RequireRefresh = true;
							consume = true;
						}
						break;
					case SDLK_BACKSPACE:
						if (m_InsertDeleteEnabled)
						{
							DoBackspace(inKeyboard.IsModifierDownExclusive(Keyboard::Shift));
							m_RequireRefresh = true;
							consume = true;
						}
						break;
					case SDLK_RETURN:
						if (inKeyboard.IsModifierDownExclusive(Keyboard::Shift))
						{
							DoEditBits(inComponentsManager);
							consume = true;
						}
						else if (inKeyboard.IsModifierDownExclusive(Keyboard::Control))
						{
							DoAction();
							m_RequireRefresh = true;
							consume = true;
						}
						break;
					case SDLK_HOME:
						SetRow(0);
						m_RequireRefresh = true;
						break;
					case SDLK_END:
						{
							int first_unused_row = GetLastUnusedRow();

							if (m_CursorY == first_unused_row)
								SetRow(m_MaxCursorY);
							else
								SetRow(first_unused_row);

							m_RequireRefresh = true;
						}
						break;
					case SDLK_PAGEDOWN:
						SetRow(m_CursorY + m_Dimensions.m_Height);
						m_RequireRefresh = true;
						break;
					case SDLK_PAGEUP:
						SetRow(m_CursorY - m_Dimensions.m_Height);
						m_RequireRefresh = true;
						break;
					default:
						if (inKeyboard.IsModifierEmpty())
						{
							char value_key = KeyboardUtils::FilterHexDigit(key_event);
							if (value_key != 0)
							{
								ApplyValueKey(value_key);

								m_CursorX = m_CursorX < m_MaxCursorX ? (m_CursorX + 1) : m_MaxCursorX;
								m_RequireRefresh = true;
								consume = true;
							}
						}
						break;
					}
				}

				AdjustVisibleArea();

				// Apply the cursor position to the cursor control
				ApplyCursorPosition(inCursorControl);

				for (const auto& key_event : inKeyboard.GetKeyPressedList())
				{
					if (key_event == SDLK_SPACE)
						OnSpaceBarPressed(true, inKeyboard.IsModifierDownExclusive(Keyboard::Control));
				}
				for (const auto& key_event : inKeyboard.GetKeyReleasedList())
				{
					if (key_event == SDLK_SPACE)
						OnSpaceBarPressed(false, false);
				}
			}
		}

		return consume;
	}


	bool ComponentTableRowElements::ConsumeInput(const Foundation::Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		if (inMouse.IsButtonPressed(Mouse::Left) && ContainsPosition(inMouse.GetPosition()))
		{
			Foundation::Point local_cell_position = GetLocalCellPosition(inMouse.GetPosition());

			if (local_cell_position.m_X >= 0 && local_cell_position.m_X < m_Dimensions.m_Width)
			{
				const int column_pos = local_cell_position.m_X - 4;

				if (column_pos >= 0)
				{
					const int column = column_pos / 3;
					const int column_nubble = column_pos - (column * 3);

					m_CursorX = (column << 1) + (column_nubble == 0 ? 0 : 1);
				}
			}
			if(local_cell_position.m_Y >= 0 && local_cell_position.m_Y < m_Dimensions.m_Height)
			{
				SetRow(m_TopRow + local_cell_position.m_Y);

				ApplyCursorPosition(inCursorControl);
			}

			m_RequireRefresh = true;
			return true;
		}

		if (m_HasControl)
			DoScrollWheel(inMouse, &inCursorControl);

		return false;
	}


	bool ComponentTableRowElements::ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse)
	{
		if (!m_HasControl)
			DoScrollWheel(inMouse, nullptr);

		return false;
	}


	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentTableRowElements::Refresh(const DisplayState& inDisplayState)
	{
		using namespace Utility;

		if (m_RequireRefresh)
		{
			const bool is_uppercase = inDisplayState.IsHexUppercase();
			const unsigned int index_multiplier = m_IndexAsContinuousMemory ? m_DataSource->GetColumnCount() : 1;

			int y = m_Position.m_Y;
			unsigned int row = m_TopRow;
			unsigned int cursor_y = static_cast<unsigned int>(m_CursorY);

			for (int i = 0; i < m_Dimensions.m_Height; i++)
			{
				TextColoring base_text_coloring(ToColor(m_HasControl && cursor_y == row ? UserColor::TableHighlight : UserColor::TableDefault));
				TextColoring index_text_coloring(ToColor(cursor_y == row ? UserColor::TableHighlight : UserColor::TableDefault));

				m_TextField->PrintHexValue(m_Position.m_X, y, index_text_coloring, is_uppercase, static_cast<unsigned char>(row * index_multiplier));
				m_TextField->PrintChar(m_Position.m_X + 2, y, index_text_coloring, ':');

				if (row < m_DataSource->GetRowCount())
				{
					int base_data_offset = row * m_DataSource->GetColumnCount();

					if (m_HighlightRowOfSelectedSongIndex)
					{
						//if(row == )
					}

					TextColoring text_coloring = GetTextColoring(base_data_offset, base_text_coloring);

					for (unsigned int j = 0; j < m_DataSource->GetColumnCount(); j++)
					{
						unsigned char value = (*m_DataSource)[base_data_offset + j];
						m_TextField->PrintHexValue(m_Position.m_X + 4 + (j * 3), y, text_coloring, is_uppercase, value);
					}
				}

				++row;
				++y;
			}

			m_RequireRefresh = false;
		}
	}


	void ComponentTableRowElements::HandleDataChange()
	{
		if (m_HasDataChange)
		{
			if(m_DataSource->PushDataToSource())
			{
				m_HasDataChange = false;

				if (m_HasLocalDataChange)
				{
					AddUndoRecentModificationStep(false);
					m_HasLocalDataChange = false;
				}
			}
		}
	}


	void ComponentTableRowElements::PullDataFromSource(const bool inFromUndo)
	{
		m_DataSource->PullDataFromSource();
	}


	void ComponentTableRowElements::ExecuteInsertDeleteRule(const DriverInfo::TableInsertDeleteRule& inRule, int inSourceTableID, int inIndexPre, int inIndexPost)
	{
		FOUNDATION_ASSERT(inRule.m_TargetTableID == m_ComponentID);
		FOUNDATION_ASSERT(inRule.m_EvaluationCellIndex < m_DataSource->GetColumnCount());
		FOUNDATION_ASSERT(inRule.m_TargetCellIndex < m_DataSource->GetColumnCount());

		const int add_value = inIndexPost - inIndexPre;

		for (unsigned int row = 0; row < m_DataSource->GetRowCount(); ++row)
		{
			const int offset = row * m_DataSource->GetColumnCount();

			const unsigned char validation_value = (*m_DataSource)[offset + inRule.m_EvaluationCellIndex];
			const unsigned char validation_value_masked = validation_value & inRule.m_EvaluationCellMask;

			if (inRule.m_EvaluationCellMask == 0 || validation_value_masked == inRule.m_EvaluationCellConditionalValue)
			{
				const unsigned char value = (*m_DataSource)[offset + inRule.m_TargetCellIndex];

				if ((add_value > 0 && value > inIndexPre) || (add_value < 0 && value >= inIndexPre))
				{
					(*m_DataSource)[offset + inRule.m_TargetCellIndex] = static_cast<unsigned char>(value + add_value);
					m_RequireRefresh = true;
					m_HasDataChange = true;
				}
			}
		}
	}


	void ComponentTableRowElements::ExecuteAction(int inActionInput)
	{
		if (inActionInput >= 0 && inActionInput <= m_MaxCursorY)
		{
			if (m_IndexAsContinuousMemory)
				inActionInput /= m_DataSource->GetColumnCount();

			SetRow(inActionInput);

			AdjustVisibleArea();
			m_RequireRefresh = true;
		}
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentTableRowElements::DoScrollWheel(const Foundation::Mouse& inMouse, CursorControl* inCursorControl)
	{
		Point scroll_wheel = inMouse.GetWheelDelta();

		if (scroll_wheel.m_Y != 0)
		{
			Foundation::Point screen_position = inMouse.GetPosition();
			if (ContainsPosition(screen_position))
			{
				const int visible_row_count = m_Dimensions.m_Height - 1;
				const int max_top_row = m_MaxCursorY - visible_row_count;

				int top_row = m_TopRow;
				int change = scroll_wheel.m_Y;

				while (change > 0)
				{
					//--cursor_y;
					--top_row;
					--change;
				}
				while (change < 0)
				{
					//++cursor_y;
					++top_row;
					++change;
				}

				//cursor_y = cursor_y < 0 ? 0 : cursor_y > m_MaxCursorY ? m_MaxCursorY : cursor_y;
				top_row = top_row < 0 ? 0 : top_row > max_top_row ? max_top_row : top_row;

				if (top_row != m_TopRow)
				{
					m_TopRow = top_row;
					
					if(m_CursorY < top_row)
						SetRow(top_row);
					if (m_CursorY > top_row + visible_row_count)
						SetRow(top_row + visible_row_count);

					if(inCursorControl != nullptr)
						ApplyCursorPosition(*inCursorControl);

					m_RequireRefresh = true;
				}
			}
		}
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentTableRowElements::AdjustVisibleArea()
	{
		// Move view if the cursor is outside of it
		if (m_CursorY < static_cast<int>(m_TopRow))
			m_TopRow = m_CursorY;
		if (m_CursorY > static_cast<int>(m_TopRow + m_Dimensions.m_Height - 1))
			m_TopRow = m_CursorY - m_Dimensions.m_Height + 1;
	}


	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentTableRowElements::ApplyValueKey(char inValueKey)
	{
		FOUNDATION_ASSERT(m_DataSource != nullptr);

		unsigned char value = [&inValueKey]()
		{
			if (inValueKey >= '0' && inValueKey <= '9')
				return static_cast<unsigned char>(inValueKey - '0');
			if (inValueKey >= 'a' && inValueKey <= 'f')
				return static_cast<unsigned char>(10 + inValueKey - 'a');

			return static_cast<unsigned char>(0xff);
		}();

		if (value != 0xff)
		{
			AddUndoStep();

			const int data_index = m_CursorY * m_DataSource->GetColumnCount() + (m_CursorX >> 1);
			const bool bottom_part = (m_CursorX & 1) != 0;

			const int shift_value = bottom_part ? 0 : 4;
			const int data_mask = 0xf0;

			unsigned char data = (*m_DataSource)[data_index];
			data &= data_mask >> shift_value;
			data |= value << shift_value;
			(*m_DataSource)[data_index] = data;

			m_HasDataChange = true;
		}
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentTableRowElements::ApplyCursorPosition(CursorControl& inCursorControl)
	{
		int actual_cursor_x = m_Position.m_X + 4 + m_CursorX + (m_CursorX >> 1);
		int actual_cursor_y = m_Position.m_Y + m_CursorY;

		inCursorControl.SetPosition(CursorControl::Position({ actual_cursor_x, actual_cursor_y - static_cast<int>(m_TopRow) }));
	}

	void ComponentTableRowElements::DoInsert()
	{
		AddUndoStep();

		int source_index = m_CursorY * m_DataSource->GetColumnCount();
		int destination_index = source_index + m_DataSource->GetColumnCount();

		if (destination_index < m_DataSource->GetSize())
		{
			const int cell_count_to_move = m_DataSource->GetSize() - destination_index;

			for (int i = cell_count_to_move - 1; i >= 0; --i)
				(*m_DataSource)[destination_index + i] = (*m_DataSource)[source_index + i];
		}

		for (unsigned int i = 0; i < m_DataSource->GetColumnCount(); ++i)
			(*m_DataSource)[source_index + i] = 0;

		m_HasDataChange = true;

		// Raise the callback
		if (m_InsertDeleteCallback)
		{
			const unsigned int multiplier = m_IndexAsContinuousMemory ? m_DataSource->GetColumnCount() : 1;
			m_InsertDeleteCallback(m_ComponentID, m_CursorY * multiplier, (m_CursorY + 1) * multiplier);
		}
	}

	void ComponentTableRowElements::DoDelete()
	{
		AddUndoStep();

		int destination_index = m_CursorY * m_DataSource->GetColumnCount();
		int source_index = destination_index + m_DataSource->GetColumnCount();

		if (source_index < m_DataSource->GetSize())
		{
			const int cell_count_to_move = m_DataSource->GetSize() - source_index;

			for (int i = 0; i < cell_count_to_move; ++i)
				(*m_DataSource)[destination_index + i] = (*m_DataSource)[source_index + i];
		}

		for (unsigned int i = 1; i <= m_DataSource->GetColumnCount(); ++i)
			(*m_DataSource)[m_DataSource->GetSize() - i] = 0;

		m_HasDataChange = true;

		// Raise the callback
		if (m_InsertDeleteCallback)
		{
			const unsigned int multiplier = m_IndexAsContinuousMemory ? m_DataSource->GetColumnCount() : 1;
			m_InsertDeleteCallback(m_ComponentID, (m_CursorY + 1) * multiplier, m_CursorY * multiplier);
		}
	}


	void ComponentTableRowElements::DoBackspace(bool inIsShiftDown)
	{
		if (inIsShiftDown)
			DoInsert();
		else if (m_CursorY > 0)
		{
			SetRow(m_CursorY - 1);
			DoDelete();
		}
	}


	void ComponentTableRowElements::DoAction()
	{
		if (m_ActionRuleExecutionCallback)
		{
			for (const DriverInfo::TableActionRule& action_rule : m_ActionRules.m_Rules)
			{
				if ((m_CursorX >> 1) == action_rule.m_ApplicableCell || action_rule.m_ApplicableCell >= 0x80)
				{
					const int index = m_CursorY * m_DataSource->GetColumnCount();
					const unsigned char conditional_mask = action_rule.m_EvaluationCellMask;
					const unsigned char conditional_value = (*m_DataSource)[index + action_rule.m_EvaluationCellIndex];

					if (conditional_mask == 0 || (conditional_value & conditional_mask) == action_rule.m_EvaluationCellConditionalValue)
					{
						const unsigned char target_id = action_rule.m_TargetTableID == 0xff ? m_ComponentID : action_rule.m_TargetTableID;
						const unsigned char target_index = (*m_DataSource)[index + action_rule.m_TargetIndexCell] & action_rule.m_TargetIndexMask;

						m_ActionRuleExecutionCallback(target_id, target_index);
					}
				}
			}
		}
	}


	void ComponentTableRowElements::DoEditBits(ComponentsManager& inComponentsManager)
	{
		const int data_index = m_CursorY * m_DataSource->GetColumnCount() + (m_CursorX >> 1);
		unsigned char data = (*m_DataSource)[data_index];

		inComponentsManager.StartDialog
		(
			std::make_shared<DialogBitInput>(
				data,
				[this, data_index](unsigned char inReturnValue)
				{
					if ((*m_DataSource)[data_index] != inReturnValue)
					{
						AddUndoStep();
						(*m_DataSource)[data_index] = inReturnValue;
						m_HasDataChange = true;
					}
				},
				[]() {}
			)
		);
	}


	void ComponentTableRowElements::AddUndoStep()
	{
		std::shared_ptr<UndoComponentDataTableRowElements> undo_data = std::make_shared<UndoComponentDataTableRowElements>();

		undo_data->m_ComponentGroupID = m_ComponentGroupID;
		undo_data->m_ComponentID = m_ComponentID;
		undo_data->m_TopRow = m_TopRow;
		undo_data->m_CursorX = m_CursorX;
		undo_data->m_CursorY = m_CursorY;

		m_Undo->AddUndo(undo_data, [this](const UndoComponentData& inData, CursorControl& inCursorControl) { this->OnUndo(inData, inCursorControl); });

		m_HasLocalDataChange = true;
	}


	void ComponentTableRowElements::AddUndoRecentModificationStep(bool inLockCPU)
	{
		std::shared_ptr<UndoComponentDataTableRowElements> undo_data = std::make_shared<UndoComponentDataTableRowElements>();

		undo_data->m_ComponentGroupID = m_ComponentGroupID;
		undo_data->m_ComponentID = m_ComponentID;
		undo_data->m_TopRow = m_TopRow;
		undo_data->m_CursorX = m_CursorX;
		undo_data->m_CursorY = m_CursorY;

		m_Undo->AddMostRecentEdit(inLockCPU, undo_data, [this](const UndoComponentData& inData, CursorControl& inCursorControl) { this->OnUndo(inData, inCursorControl); });
	}

	
	void ComponentTableRowElements::OnUndo(const UndoComponentData& inData, CursorControl& inCursorControl)
	{
		const UndoComponentDataTableRowElements& undo_data = static_cast<const UndoComponentDataTableRowElements&>(inData);

		m_TopRow = undo_data.m_TopRow;
		m_CursorX = undo_data.m_CursorX;
		m_CursorY = undo_data.m_CursorY;

		//PullDataFromSource();

		m_RequireRefresh = true;
	}


	void ComponentTableRowElements::OnSpaceBarPressed(bool inPressed, bool inIsControlPressed)
	{
		if (m_SpaceBarPressed != inPressed)
		{
			m_SpaceBarPressed = inPressed;
		
			if (m_SpaceBarCallback)
				m_SpaceBarCallback(inPressed, inIsControlPressed);
		}
	}


	int ComponentTableRowElements::GetLastUnusedRow()
	{
		int row = m_MaxCursorY;
		int selected_row = row;

		const unsigned int column_count = m_DataSource->GetColumnCount();

		while (row >= 0)
		{
			int offset = row * column_count;

			for (unsigned int i = 0; i < column_count; ++i)
			{
				if ((*m_DataSource)[offset + i] != 0)
					return selected_row;
			}

			selected_row = row;
			--row;
		}

		return 0;
	}



	void ComponentTableRowElements::SetRow(int inRow)
	{
		if (inRow > m_MaxCursorY)
			inRow = m_MaxCursorY;
		if (inRow < 0)
			inRow = 0;

		if (m_CursorY != inRow)
		{
			m_CursorY = inRow;
			m_SelectedRowChangedEvent.Execute(m_CursorY);
		}
	}


	TextColoring ComponentTableRowElements::GetTextColoring(int inDataOffset, const TextColoring& inBaseTextColoring) const
	{
		using namespace Utility;

		for (const auto& color_rule : m_ColorRules.m_Rules)
		{
			const unsigned char value = (*m_DataSource)[inDataOffset + color_rule.m_EvaluationCellIndex];
			const unsigned char masked_value = value & color_rule.m_EvaluationCellMask;

			if (masked_value == color_rule.m_EvaluationCellConditionalValue)
			{
				if (color_rule.m_BackgroundColor < 0x80)
					return TextColoring(static_cast<Color>(color_rule.m_BackgroundColor));

				return TextColoring(ToColor(static_cast<UserColor>(static_cast<unsigned short>(color_rule.m_BackgroundColor) - 0x40)));
			}
		}

		return inBaseTextColoring;
	}
}