#include "runtime/editor/components/component_orderlistoverview.h"
#include "runtime/editor/components/utils/text_editing_data_source_table_text.h"
#include "runtime/editor/components/utils/orderlist_utils.h"
#include "runtime/editor/cursor_control.h"
#include "runtime/editor/display_state.h"
#include "runtime/editor/datasources/datasource_orderlist.h"
#include "runtime/editor/datasources/datasource_sequence.h"
#include "runtime/editor/datasources/datasource_table_text.h"
#include "runtime/editor/undo/undo.h"
#include "runtime/editor/undo/undo_componentdata/undo_componentdata_table_text.h"
#include "runtime/editor/datacopy/copypaste.h"
#include "runtime/editor/datacopy/datacopy_orderlist.h"
#include "runtime/editor/undo/undo_componentdata/undo_componentdata_orderlist_overview.h"
#include "foundation/graphics/textfield.h"
#include "foundation/input/keyboard.h"
#include "foundation/input/mouse.h"
#include "foundation/base/assert.h"
#include "utils/keyhook.h"
#include "utils/keyhookstore.h"
#include "utils/usercolors.h"
#include "utils/logging.h"

using namespace Foundation;
using namespace Utility;

namespace Editor
{
	const int ComponentOrderListOverview::ms_MarginWidth = 1;
	const int ComponentOrderListOverview::ms_TextWidth = 19;

	int ComponentOrderListOverview::GetWidthFromChannelCount(int inChannelCount)
	{
		// Example, 3 rows: " xxxx: 01 02 03 abcdef0123456789 "
		return ms_MarginWidth + 6 + 3 * inChannelCount + ms_TextWidth;
	}

	int ComponentOrderListOverview::GetOutputPositionFromCursorX(int inChannel)
	{
		return ms_MarginWidth + 6 + 3 * inChannel;
	}


	ComponentOrderListOverview::ComponentOrderListOverview(
		int inID,
		int inGroupID, 
		Undo* inUndo,
		TextField* inTextField,
		const Utility::KeyHookStore& inKeyHookStore,
		std::shared_ptr<DataSourceTableText> inDataSourceTableText,
		std::vector<std::shared_ptr<DataSourceOrderList>>& inOrderLists,
		const std::vector<std::shared_ptr<DataSourceSequence>>& inSequenceList,
		int inX,
		int inY,
		int inHeight,
		std::function<void(int, bool)> inSetTrackEventPosFunction
	)
		: ComponentBase(inID, inGroupID, inUndo, inTextField, inX, inY, ComponentOrderListOverview::GetWidthFromChannelCount(static_cast<int>(inOrderLists.size())), inHeight)
		, m_TableText(inDataSourceTableText)
		, m_OrderLists(inOrderLists)
		, m_SequenceList(inSequenceList)
		, m_PlaybackEventPosition(-1)
		, m_CursorX(0)
		, m_CursorY(0)
		, m_MaxCursorY(0)
		, m_MaxCursorX(static_cast<int>(inOrderLists.size()))
		, m_IsMarkingArea(false)
		, m_TopPosition(0)
		, m_SetTrackEventPosFunction(inSetTrackEventPosFunction)
		, m_InvokeOrderListChangedEventChannel(-1)
	{
		m_TextEditingDataSourceTableText = std::make_unique<TextEditingDataSourceTableText>(
			inUndo, 
			inID,
			inGroupID,
			inDataSourceTableText, 
			ms_TextWidth,
			true);

		ConfigureKeyHooks(inKeyHookStore);
	}


	ComponentOrderListOverview::~ComponentOrderListOverview()
	{

	}


	void ComponentOrderListOverview::SetHasControl(GetControlType inGetControlType, CursorControl& inCursorControl)
	{
		// Set has control, but do not enaBle the cursor for this component, because it doesn't need it yet!
		m_HasControl = true;
		m_RequireRefresh = true;
	}



	bool ComponentOrderListOverview::ConsumeInput(const Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		bool consume = false;

		if (IsEditingText())
			consume = m_TextEditingDataSourceTableText->ConsumeKeyboardInput(inKeyboard, inCursorControl);
		else
		{
			const bool isNoModifierDown = inKeyboard.IsModifierEmpty();
			const bool isOnlyShiftDown = inKeyboard.IsModifierDownExclusive(Keyboard::Shift);

			for (const auto& key_event : inKeyboard.GetKeyEventList())
			{
				if (Utility::ConsumeInputKeyHooks(key_event, inKeyboard.GetModiferMask(), m_KeyHooks, KeyHookContext({ inComponentsManager })))
				{
					consume = true;
					continue;
				}

				switch (key_event)
				{
				case SDLK_DOWN:
					if (inKeyboard.IsModifierEmpty())
					{
						if (m_IsMarkingArea)
							DoCancelMarking();

						if (DoCursorDown(1))
						{
							m_RequireRefresh = true;
							consume = true;
						}
					}
					else if (isOnlyShiftDown && m_CursorX != m_MaxCursorX)
					{
						if (!m_IsMarkingArea)
							DoBeginMarking();

						if (DoCursorDown(1))
						{
							m_MarkingToY = m_CursorY;
							m_RequireRefresh = true;
							consume = true;
						}
					}
					break;
				case SDLK_UP:
					if (inKeyboard.IsModifierEmpty())
					{
						if (m_IsMarkingArea)
							DoCancelMarking();

						if (DoCursorUp(1))
						{
							m_RequireRefresh = true;
							consume = true;
						}
					}
					else if (isOnlyShiftDown && m_CursorX != m_MaxCursorX)
					{
						if (!m_IsMarkingArea)
							DoBeginMarking();

						if (DoCursorUp(1))
						{
							m_MarkingToY = m_CursorY;
							m_RequireRefresh = true;
							consume = true;
						}
					}
					break;
				case SDLK_LEFT:
					if(inKeyboard.IsModifierEmpty())
					{
						if (m_IsMarkingArea)
							DoCancelMarking();

						DoCursorLeft();

						m_RequireRefresh = true;
						consume = true;
					}
					break;
				case SDLK_RIGHT:
					if (inKeyboard.IsModifierEmpty())
					{
						if (m_IsMarkingArea)
							DoCancelMarking();

						DoCursorRight();

						m_RequireRefresh = true;
						consume = true;
					}
					break;
				case SDLK_PAGEDOWN:
					if (m_IsMarkingArea)
						DoCancelMarking();

					if (DoCursorDown(20))
					{
						m_RequireRefresh = true;
						consume = true;
					}
					break;
				case SDLK_PAGEUP:
					if (m_IsMarkingArea)
						DoCancelMarking();

					if (DoCursorUp(20))
					{
						m_RequireRefresh = true;
						consume = true;
					}
					break;
				case SDLK_HOME:
					if (m_IsMarkingArea)
						DoCancelMarking();

					if (DoHome())
					{
						m_RequireRefresh = true;
						consume = true;
					}
					break;
				case SDLK_END:
					if (m_IsMarkingArea)
						DoCancelMarking();

					if (DoEnd())
					{
						m_RequireRefresh = true;
						consume = true;
					}
					break;
				case SDLK_RETURN:
					if (m_IsMarkingArea)
						DoCancelMarking();

					if (inKeyboard.IsModifierDown(Keyboard::Shift) || inKeyboard.IsModifierDown(Keyboard::Control))
					{
						if (m_CursorY >= 0 && m_CursorY < static_cast<int>(m_Overview.size()) && m_SetTrackEventPosFunction)
						{
							m_SetTrackEventPosFunction(m_Overview[m_CursorY].m_EventPos, inKeyboard.IsModifierDownExclusive(Keyboard::Control));
							consume = true;
						}
					}
					else if (isNoModifierDown)
					{
						DoStartEditText(inCursorControl);
						m_RequireRefresh = true;
						consume = true;
					}
					break;
				case SDLK_BACKSPACE:
					if (m_CursorX == m_MaxCursorX)
					{
						if (m_IsMarkingArea)
							DoCancelMarking();

						if (isNoModifierDown)
						{
							if (m_CursorY > 0)
							{
								DoCursorUp(1);
								DoDeleteTextRow(m_CursorY);

								m_RequireRefresh = true;
								consume = true;
							}
						}
						else if (isOnlyShiftDown)
						{
							DoInsertTextRow(m_CursorY);

							m_RequireRefresh = true;
							consume = true;
						}
					}
					break;
				case SDLK_DELETE:
					if (m_CursorX == m_MaxCursorX)
					{
						if (m_IsMarkingArea)
							DoCancelMarking();

						if (isNoModifierDown)
						{
							if (m_CursorY >= 0)
							{
								DoDeleteTextRow(m_CursorY);

								m_RequireRefresh = true;
								consume = true;
							}
						}
					}
					break;
				case SDLK_INSERT:

					if (m_CursorX == m_MaxCursorX)
					{
						if (m_IsMarkingArea)
							DoCancelMarking();

						if (isNoModifierDown)
						{
							DoInsertTextRow(m_CursorY);

							m_RequireRefresh = true;
							consume = true;
						}
					}
					break;
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


	bool ComponentOrderListOverview::ConsumeInput(const Foundation::Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		bool consume = false;

		if (ContainsPosition(inMouse.GetPosition()))
		{
			if (inMouse.IsButtonDoublePressed(Mouse::Left))
			{
				Point cell_position = GetLocalCellPosition(inMouse.GetPosition());
				m_CursorY = cell_position.m_Y + m_TopPosition;

				if (m_CursorY >= 0 && m_CursorY < static_cast<int>(m_Overview.size()) && m_SetTrackEventPosFunction)
					m_SetTrackEventPosFunction(m_Overview[m_CursorY].m_EventPos, true);

				m_RequireRefresh = true;

				consume = true;
			}
			else if (inMouse.IsButtonPressed(Mouse::Left))
			{
				Point local_cell_position = GetLocalCellPosition(inMouse.GetPosition());
				m_CursorY = local_cell_position.m_Y + m_TopPosition;

				if (m_CursorY >= 0 && m_CursorY < static_cast<int>(m_Overview.size()) && m_SetTrackEventPosFunction)
					m_SetTrackEventPosFunction(m_Overview[m_CursorY].m_EventPos, false);

				m_RequireRefresh = true;

				const int text_x = m_Dimensions.m_Width - ms_TextWidth;
				const int text_x_right = m_Dimensions.m_Width;

				if (local_cell_position.m_X >= text_x && local_cell_position.m_X <= text_x_right)
				{
					if (IsEditingText() && m_CursorY != m_TextEditingDataSourceTableText->GetTextLineIndex())
						DoStopEditText(inCursorControl, false);
					if (m_IsMarkingArea)
						DoCancelMarking();

					m_CursorX = m_MaxCursorX;

					if (m_CursorY <= m_MaxCursorY)
					{
						DoStartEditText(inCursorControl);

						const int cursor_pos = local_cell_position.m_X - text_x;
						m_TextEditingDataSourceTableText->TrySetCursorPosition(cursor_pos);
					}
				}
				else
				{
					if (IsEditingText())
						DoStopEditText(inCursorControl, false);
					if (m_IsMarkingArea)
						DoCancelMarking();

					for (int i = m_MaxCursorX; i >= 0; --i)
					{
						int x = GetOutputPositionFromCursorX(i);
						if (local_cell_position.m_X >= x)
						{
							m_CursorX = i;
							break;
						}
					}
				}

				consume = true;
			}
		}
		
		if (!IsEditingText())
			DoMouseWheel(inMouse);

		return consume;
	}


	bool ComponentOrderListOverview::ConsumeNonExclusiveInput(const Mouse& inMouse)
	{
		if (!m_HasControl)
		{
			DoMouseWheel(inMouse);
		}

		return false;
	}



	void ComponentOrderListOverview::Refresh(const DisplayState& inDisplayState)
	{
		if (m_RequireRefresh)
		{
			RebuildOverview();

			m_TextField->Clear(m_Rect);
			
			Rect ListRect = m_Rect;
			ListRect.m_Dimensions.m_Width -= ms_TextWidth;

			Rect TextRect = m_Rect;
			TextRect.m_Position.m_X += ListRect.m_Dimensions.m_Width;
			TextRect.m_Dimensions.m_Width -= ListRect.m_Dimensions.m_Width;
			
			m_TextField->ColorAreaBackground(ToColor(UserColor::SongListBackground), ListRect);
			m_TextField->ColorAreaBackground(ToColor(UserColor::SongListBackgroundText), TextRect);
			m_TextField->ColorArea(ToColor(UserColor::SongListText), TextRect);

			const bool is_uppercase = inDisplayState.IsHexUppercase();
			const int cursor_position = m_CursorY - m_TopPosition;

			if (m_IsMarkingArea)
			{
				const int top_marking = GetMarkingTopY() - m_TopPosition;
				const int bottom_marking = GetMarkingBottomY() - m_TopPosition;

				if (bottom_marking >= 0 && top_marking < m_Rect.m_Dimensions.m_Height)
				{
					const int adjusted_top_marking = top_marking < 0 ? 0 : top_marking;
					const int adjusted_bottom_marking = bottom_marking > m_Rect.m_Dimensions.m_Height ? m_Rect.m_Dimensions.m_Height : bottom_marking;

					const int marking_height = adjusted_bottom_marking - adjusted_top_marking;

					Rect marking_rect = m_Rect;

					marking_rect.m_Position.m_X += GetOutputPositionFromCursorX(m_MarkingX);
					marking_rect.m_Position.m_Y += adjusted_top_marking;
					marking_rect.m_Dimensions = { 2, marking_height };

					m_TextField->ColorAreaBackground(ToColor(m_HasControl ? UserColor::SongListCursorFocus : UserColor::SongListCursorDefault), marking_rect);
				}
			}

			if (cursor_position >= 0 && cursor_position < m_Dimensions.m_Height)
			{
				m_TextField->ColorAreaBackground(ToColor(UserColor::SongListCursorDefault), { m_Position + Point(0, cursor_position), { m_Dimensions.m_Width, 1 } });

				if (m_HasControl)
				{
					bool isTextField = m_CursorX == m_MaxCursorX;

					Point cursor_highlight_position = { GetOutputPositionFromCursorX(m_CursorX), cursor_position };
					Extent cursor_highlight_extent = { isTextField ? ms_TextWidth : 2, 1 };

					m_TextField->ColorAreaBackground(ToColor(UserColor::SongListCursorFocus), { m_Position + cursor_highlight_position, cursor_highlight_extent });
				
					Point highlight_position = { 0, cursor_position };
					Extent highlight_extent = { ms_MarginWidth + 5, 1 };

					m_TextField->ColorAreaBackground(ToColor(UserColor::SongListCursorFocus), { m_Position + highlight_position, highlight_extent });
				}
			}

			int local_y = 0;
			int overview_list_size = static_cast<int>(m_Overview.size());

			Color event_pos_values = ToColor(UserColor::SongListEventPos);

			for (int i = m_TopPosition; i < overview_list_size && local_y < m_Dimensions.m_Height; ++i)
			{
				OverviewEntry& entry = m_Overview[i];

				int x = m_Position.m_X + 1;
				int y = local_y + m_Position.m_Y;

				m_TextField->PrintHexValue(x, y, event_pos_values, is_uppercase, static_cast<unsigned short>(entry.m_EventPos));

				const int next_event_pos = i < overview_list_size - 1 ? m_Overview[i + 1].m_EventPos : 0x7fffffff;

				if(m_PlaybackEventPosition >= 0 && m_PlaybackEventPosition >= entry.m_EventPos && m_PlaybackEventPosition < next_event_pos)
					m_TextField->ColorAreaBackground(ToColor(UserColor::SongListPlaybackMarker), Rect({ { x, y}, { 4, 1 } }));

				m_TextField->Print(x + 4, y, event_pos_values, ":");

				x += 6;

				for (auto& sequence_entry : entry.m_SequenceEntries)
				{
					int sequence_index = sequence_entry.m_Index;
					if (sequence_index >= 0)
					{
						const Color color = sequence_index < 0x100 ? ToColor(UserColor::SongListValues) : ToColor(UserColor::SongListLoopMarker);
						m_TextField->PrintHexValue(x, y, color, is_uppercase, static_cast<unsigned char>(sequence_index & 0x0ff));
					}

					x += 3;
				}

				int text_x = m_Position.m_X + ms_MarginWidth + 5 + 3 * m_OrderLists.size() + 1;

				if (i < m_TableText->GetSize())
				{
					const bool is_editing_line = (m_CursorY == i) && IsEditingText();
					m_TextField->Print(text_x, y, is_editing_line ? ToColor(UserColor::SongListTextEditing) : ToColor(UserColor::SongListText), (*m_TableText)[i], ms_TextWidth);
				}

				++local_y;
			}

			m_RequireRefresh = false;
		}
	}


	bool ComponentOrderListOverview::HasDataChange() const
	{
		if (m_TextEditingDataSourceTableText->HasDataChange())
			return true;

		return ComponentBase::HasDataChange();
	}


	void ComponentOrderListOverview::HandleDataChange()
	{
		if (m_TextEditingDataSourceTableText->HasDataChange())
			m_TextEditingDataSourceTableText->ApplyDataChange();

		m_HasDataChange = false;

		if (m_InvokeOrderListChangedEventChannel >= 0)
		{
			m_OrderListChangedEvent.Execute(static_cast<unsigned int>(m_InvokeOrderListChangedEventChannel));
			m_InvokeOrderListChangedEventChannel = -1;
		}
	}

	
	void ComponentOrderListOverview::PullDataFromSource(const bool inFromUndo)
	{
		m_TableText->PullDataFromSource();
	}


	bool ComponentOrderListOverview::IsNoteInputSilenced() const
	{
		return IsEditingText();
	}


	bool ComponentOrderListOverview::IsFastForwardAllowed() const
	{
		return !IsEditingText();
	}

	void ComponentOrderListOverview::ClearHasControl(CursorControl& inCursorControl)
	{
		if(IsEditingText())
			DoStopEditText(inCursorControl, false);

		ComponentBase::ClearHasControl(inCursorControl);
	}

	void ComponentOrderListOverview::ExecuteInsertDeleteRule(const DriverInfo::TableInsertDeleteRule& inRule, int inSourceTableID, int inIndexPre, int inIndexPost)
	{

	}


	void ComponentOrderListOverview::ExecuteAction(int inActionInput)
	{

	}


	void ComponentOrderListOverview::TellPlaybackEventPosition(int inPlaybackEventPosition)
	{
		if (m_PlaybackEventPosition != inPlaybackEventPosition)
		{
			m_PlaybackEventPosition = inPlaybackEventPosition;
			m_RequireRefresh = true;
		}
	}


	ComponentOrderListOverview::OrderListChangedEvent& ComponentOrderListOverview::GetOrderListChangedEvent()
	{
		return m_OrderListChangedEvent;
	}



	void ComponentOrderListOverview::DoMouseWheel(const Foundation::Mouse& inMouse)
	{
		Point scroll_wheel = inMouse.GetWheelDelta();

		if (scroll_wheel.m_Y != 0)
		{
			Point screen_position = inMouse.GetPosition();
			if (ContainsPosition(screen_position))
			{
				const int visible_height = m_Dimensions.m_Height - 1;
				const int max_top_pos = m_MaxCursorY < visible_height ? 0 : (m_MaxCursorY - visible_height);

				int top_pos = m_TopPosition;
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

				if (top_pos != m_TopPosition)
				{
					m_TopPosition = top_pos;

					if (m_CursorY < m_TopPosition)
						m_CursorY = m_TopPosition;
					if (m_CursorY > m_TopPosition + visible_height)
						m_CursorY = m_TopPosition + visible_height;

					m_RequireRefresh = true;
				}
			}
		}
	}


	bool ComponentOrderListOverview::DoCursorUp(unsigned int inSteps)
	{
		if (m_CursorY > 0)
		{
			m_CursorY -= inSteps;

			if (m_CursorY < 0)
				m_CursorY = 0;

			if (m_CursorY < m_TopPosition)
				m_TopPosition = m_CursorY;

			return true;
		}

		return false;
	}


	bool ComponentOrderListOverview::DoCursorDown(unsigned int inSteps)
	{
		if (m_CursorY < m_MaxCursorY)
		{
			m_CursorY += inSteps;

			if (m_CursorY > m_MaxCursorY)
				m_CursorY = m_MaxCursorY;

			if (m_CursorY >= m_TopPosition + m_Dimensions.m_Height)
				m_TopPosition = m_CursorY - (m_Dimensions.m_Height - 1);

			return true;
		}

		return false;
	}


	bool ComponentOrderListOverview::DoCursorLeft()
	{
		if (m_CursorX > 0)
		{
			--m_CursorX;
			return true;
		}

		return false;
	}


	bool ComponentOrderListOverview::DoCursorRight()
	{
		if (m_CursorX < m_MaxCursorX)
		{
			++m_CursorX;
			return true;
		}

		return false;
	}


	bool ComponentOrderListOverview::DoHome()
	{
		if (m_CursorY > 0)
		{
			if (m_CursorY == m_TopPosition)
			{
				m_CursorY = 0;
				m_TopPosition = 0;
			}
			else
				m_CursorY = m_TopPosition;

			return true;
		}

		return false;
	}

	bool ComponentOrderListOverview::DoEnd()
	{
		if (m_CursorY < m_MaxCursorY)
		{
			int max_visible = m_TopPosition + m_Dimensions.m_Height - 1;

			if (max_visible <= m_MaxCursorY && m_CursorY < max_visible)
			{
				m_CursorY = max_visible;
			}
			else
			{
				m_CursorY = m_MaxCursorY;

				if (m_CursorY >= m_TopPosition + m_Dimensions.m_Height)
					m_TopPosition = m_CursorY - (m_Dimensions.m_Height - 1);
			}

			return true;
		}

		return false;
	}


	bool ComponentOrderListOverview::DoInsertTextRow(unsigned int inRow)
	{
		const unsigned int size = static_cast<unsigned int>(m_TableText->GetSize());

		if (inRow < size && size > 1)
		{
			AddUndoTextStep();

			for (int i = static_cast<int>(size) - 2; i >= static_cast<int>(inRow); --i)
				(*m_TableText)[i + 1] = (*m_TableText)[i];

			(*m_TableText)[inRow] = "";

			m_TableText->PushDataToSource();
			AddMostRecentTextEdit();

			m_HasDataChange = true;

			return true;
		}

		return false;
	}


	bool ComponentOrderListOverview::DoDeleteTextRow(unsigned int inRow)
	{
		const unsigned int size = static_cast<unsigned int>(m_TableText->GetSize());

		if (inRow < size && inRow >= 0 && size > 1)
		{
			AddUndoTextStep();

			for (unsigned int i = inRow; i < size - 1; ++i)
				(*m_TableText)[i] = (*m_TableText)[i + 1];

			(*m_TableText)[size - 1] = "";

			m_TableText->PushDataToSource();
			AddMostRecentTextEdit();
			
			m_HasDataChange = true;

			return true;
		}

		return false;
	}


	bool ComponentOrderListOverview::DoCopy()
	{
		if (!m_IsMarkingArea)
			DoBeginMarking();

		{
			const int channel_count = static_cast<int>(m_OrderLists.size());
			const int channel = m_MarkingX;

			if (channel < channel_count)
			{
				std::vector<DataSourceOrderList::Entry> order_list_copy;

				int marking_top = std::min(m_MarkingFromY, m_MarkingToY);
				int marking_bottom = std::max(m_MarkingFromY, m_MarkingToY);

				for (int i = marking_top; i <= marking_bottom; ++i)
				{
					if (static_cast<int>(m_Overview.size()) <= i)
						break;

					const auto& entry = m_Overview[i];

					std::string output;

					if (entry.m_SequenceEntries[channel].m_Index >= 0)
					{
						const auto transpose = entry.m_SequenceEntries[channel].m_Transpose;
						const auto index = entry.m_SequenceEntries[channel].m_Index;

						FOUNDATION_ASSERT(transpose < 0x100);

						// Logging
						output += std::to_string(channel + 1) + ": " + std::to_string(transpose) + " - " + std::to_string(index & 0xff);
						Logging::instance().Info(output.c_str());

						order_list_copy.push_back({ static_cast<unsigned char>(transpose), static_cast<unsigned char>(index & 0xff) });
					}
				}

				CopyPaste::Instance().SetData(new DataCopyOrderList(order_list_copy));
			}
		}

		return true;
	}


	bool ComponentOrderListOverview::DoPaste()
	{
		if (CopyPaste::Instance().HasData<DataCopyOrderList>())
		{
			const int channel_count = m_OrderLists.size();
			const int channel = m_CursorX;

			if (channel < channel_count)
			{
				int current_event_pos = m_Overview[m_CursorY].m_EventPos;

				std::shared_ptr<DataSourceOrderList>& order_list = m_OrderLists[channel];
				const int orderlist_length = order_list->GetLength();

				int order_list_destination_index = [&](const int inCurrentEventPos)
				{
					int event_pos = 0;

					for (int i = 0; i < orderlist_length; ++i)
					{
						if (inCurrentEventPos == event_pos)
							return i;

						const unsigned char current_transposition = (*order_list)[i].m_Transposition;
						if (current_transposition >= 0xfe)
							return -1;

						const unsigned char current_sequence_index = (*order_list)[i].m_SequenceIndex;
						FOUNDATION_ASSERT(current_sequence_index < 0x80);

						const int sequence_length = static_cast<int>(m_SequenceList[current_sequence_index]->GetLength());
						if (event_pos < inCurrentEventPos && event_pos + sequence_length > inCurrentEventPos)
							return i + 1;

						event_pos += sequence_length;
					}

					return orderlist_length;
				}(current_event_pos);

				if (order_list_destination_index < 0)
					return false;

				AddUndoSequenceStep(channel);

				const DataCopyOrderList& order_list_copy = *CopyPaste::Instance().GetData<DataCopyOrderList>();

				for (int i = 0; i < static_cast<int>(order_list_copy.GetEntryCount()); ++i)
				{
					if (!order_list->CanIncreaseSize())
						break;

					OrderListInsert(order_list, i + order_list_destination_index, order_list_copy[i]);					
				}

				// Insert and raise events
				m_OrderListChangedEvent.Execute(channel);

				m_RequireRefresh = true;
			}
		}

		return true;
	}


	void ComponentOrderListOverview::DoBeginMarking()
	{
		FOUNDATION_ASSERT(!m_IsMarkingArea);

		m_IsMarkingArea = true;

		m_MarkingX = m_CursorX;
		m_MarkingFromY = m_CursorY;
		m_MarkingToY = m_CursorY;
	}


	void ComponentOrderListOverview::DoCancelMarking()
	{
		FOUNDATION_ASSERT(m_IsMarkingArea);

		m_IsMarkingArea = false;
	}


	int ComponentOrderListOverview::GetMarkingTopY() const
	{
		FOUNDATION_ASSERT(m_IsMarkingArea);

		if (m_MarkingFromY < m_MarkingToY)
			return m_MarkingFromY;

		return m_MarkingToY;
	}


	int ComponentOrderListOverview::GetMarkingBottomY() const
	{
		FOUNDATION_ASSERT(m_IsMarkingArea);

		if (m_MarkingToY > m_MarkingFromY)
			return m_MarkingToY;

		return m_MarkingFromY + 1;
	}


	void ComponentOrderListOverview::AddUndoTextStep()
	{
		const int count = m_TableText->GetSize();

		std::vector<std::string> text_lines;
		for (int i = 0; i < count; ++i)
			text_lines.push_back((*m_TableText)[i]);

		auto undo_data = std::make_shared<UndoComponentDataTableText>();

		undo_data->m_ComponentID = m_ComponentID;
		undo_data->m_ComponentGroupID = m_ComponentGroupID;
		undo_data->m_TextLines = text_lines;

		m_Undo->AddUndo(undo_data, [this](const UndoComponentData& inData, CursorControl& inCursorControl) { this->OnUndoTextEdit(inData, inCursorControl); });
	}


	void ComponentOrderListOverview::AddMostRecentTextEdit()
	{
		const int count = m_TableText->GetSize();

		std::vector<std::string> text_lines;
		for (int i = 0; i < count; ++i)
			text_lines.push_back((*m_TableText)[i]);

		auto undo_data = std::make_shared<UndoComponentDataTableText>();

		undo_data->m_ComponentID = m_ComponentID;
		undo_data->m_ComponentGroupID = m_ComponentGroupID;
		undo_data->m_TextLines = text_lines;

		m_Undo->AddMostRecentEdit(true, undo_data, [this](const UndoComponentData& inData, CursorControl& inCursorControl) { this->OnUndoTextEdit(inData, inCursorControl); });
	}

	void ComponentOrderListOverview::OnUndoTextEdit(const UndoComponentData& inData, CursorControl& inCursorControl)
	{
		const UndoComponentDataTableText& undo_data = static_cast<const UndoComponentDataTableText&>(inData);

		FOUNDATION_ASSERT(undo_data.m_TextLines.size() == m_TableText->GetSize());

		const int count = m_TableText->GetSize();
		for (int i = 0; i < count; ++i)
		{
			auto& dest = (*m_TableText)[i];
			dest = undo_data.m_TextLines[i];
		}

		m_HasDataChange = true;
		m_RequireRefresh = true;
	}


	void ComponentOrderListOverview::OnUndoSequenceEdit(const UndoComponentData& inData, CursorControl& inCursorControl)
	{
		const UndoComponentDataOrderListOverview& undo_data = static_cast<const UndoComponentDataOrderListOverview&>(inData);

		m_CursorX = undo_data.m_CursorX;
		m_CursorY = undo_data.m_CursorY;
		m_TopPosition = undo_data.m_TopPosition;

		m_HasDataChange = true;
		m_RequireRefresh = true;

		m_InvokeOrderListChangedEventChannel = static_cast<int>(undo_data.m_ModifiedChannel);
	}


	void ComponentOrderListOverview::AddUndoSequenceStep(unsigned int inChannel)
	{
		std::shared_ptr<UndoComponentDataOrderListOverview> undo_data = std::make_shared<UndoComponentDataOrderListOverview>();

		undo_data->m_ComponentGroupID = m_ComponentGroupID;
		undo_data->m_ComponentID = m_ComponentID;
		undo_data->m_TopPosition = m_TopPosition;
		undo_data->m_CursorX = m_CursorX;
		undo_data->m_CursorY = m_CursorY;
		undo_data->m_ModifiedChannel = inChannel;

		m_Undo->AddUndo(undo_data, [this](const UndoComponentData& inData, CursorControl& inCursorControl) { this->OnUndoSequenceEdit(inData, inCursorControl); });
	}


	void ComponentOrderListOverview::AddMostRecentSequenceEdit()
	{
		std::shared_ptr<UndoComponentDataOrderListOverview> undo_data = std::make_shared<UndoComponentDataOrderListOverview>();

		undo_data->m_ComponentGroupID = m_ComponentGroupID;
		undo_data->m_ComponentID = m_ComponentID;
		//		undo_data->m_TopRow = m_TopRow;
		undo_data->m_CursorX = m_CursorX;
		undo_data->m_CursorY = m_CursorY;

		//		m_Undo->AddUndo(undo_data, [this](const UndoComponentData& inData, CursorControl& inCursorControl) { this->OnUndo(inData, inCursorControl); });
	}


	bool ComponentOrderListOverview::IsEditingText() const
	{
		return m_TextEditingDataSourceTableText->IsEditing();
	}


	void ComponentOrderListOverview::DoStartEditText(CursorControl& inCursorControl)
	{
		auto GetEditingRowIndex = [&]() -> unsigned int
		{
			return static_cast<unsigned int>(m_CursorY);
		};

		if (!IsEditingText())
		{
			m_TextEditingDataSourceTableText->StartEditing(GetEditingRowIndex(), GetEditingTextScreenPosition());
			inCursorControl.SetEnabled(true);
			m_RequireRefresh = true;
		}
	}


	void ComponentOrderListOverview::DoStopEditText(CursorControl& inCursorControl, bool inCancel)
	{
		m_TextEditingDataSourceTableText->StopEditing(inCancel, inCursorControl);
		m_RequireRefresh = true;
	}


	Foundation::Point ComponentOrderListOverview::GetEditingTextScreenPosition() const
	{
		return Foundation::Point(m_Position.m_X + m_Rect.m_Dimensions.m_Width - ms_TextWidth, m_Position.m_Y + m_CursorY - m_TopPosition);
	}


	void ComponentOrderListOverview::RebuildOverview()
	{
		m_Overview.clear();

		std::vector<int> orderlist_indices;
		std::vector<int> orderlist_event_pos;

		const int channel_count = static_cast<int>(m_OrderLists.size());

		for (int i = 0; i < channel_count; ++i)
		{
			orderlist_indices.push_back(0);
			orderlist_event_pos.push_back(0);
		}

		int event_pos = 0;

		while (event_pos < 0x7fffffff)
		{
			OverviewEntry entry;

			entry.m_EventPos = event_pos;

			// Construct the next entry
			for (int i = 0; i < channel_count; ++i)
			{
				const int loop_index = static_cast<int>(m_OrderLists[i]->GetLoopIndex());
				if (orderlist_event_pos[i] == event_pos)
				{
					const int orderlist_index = orderlist_indices[i];
					const auto& order_list_entry = (*m_OrderLists[i])[orderlist_index];
					bool is_end = order_list_entry.m_Transposition >= 0xfe;

					if (!is_end)
					{
						if (orderlist_index == loop_index)
							entry.m_SequenceEntries.push_back({ order_list_entry.m_Transposition, order_list_entry.m_SequenceIndex | 0x100 });
						else
							entry.m_SequenceEntries.push_back({ order_list_entry.m_Transposition, order_list_entry.m_SequenceIndex });

						orderlist_event_pos[i] += m_SequenceList[order_list_entry.m_SequenceIndex]->GetLength();
						orderlist_indices[i]++;
					}
					else
					{
						entry.m_SequenceEntries.push_back({ -1, -1 });
						orderlist_event_pos[i] = -1;
					}
				}
				else 
					entry.m_SequenceEntries.push_back({ -1, -1 });
			}

			m_Overview.push_back(entry);

			int event_pos_forward = [&]()
			{
				int closest_event_pos = 0x7fffffff;

				for (const int& next_event_pos : orderlist_event_pos)
				{
					if (next_event_pos >= 0)
					{
						if (next_event_pos < closest_event_pos)
							closest_event_pos = next_event_pos;
					}
				}

				return closest_event_pos;
			}();

			event_pos = event_pos_forward;
		}

		m_MaxCursorY = static_cast<int>(m_Overview.size()) - 1;

		if (m_CursorY > m_MaxCursorY)
			m_CursorY = m_MaxCursorY;
	}

	void ComponentOrderListOverview::ConfigureKeyHooks(const Utility::KeyHookStore& inKeyHookStore)
	{
		using namespace Utility;

		m_KeyHooks.clear();

		m_KeyHooks.push_back({ "Key.OrderListOverview.Copy", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			DoCopy();

			return true;
		} });
		m_KeyHooks.push_back({ "Key.OrderListOverview.Paste", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			DoPaste();

			return true;
		} });
	}
}
