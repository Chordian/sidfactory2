#include "component_track.h"

#include "runtime/editor/auxilarydata/auxilary_data_collection.h"
#include "runtime/editor/auxilarydata/auxilary_data_editing_preferences.h"
#include "runtime/editor/cursor_control.h"
#include "runtime/editor/display_state.h"
#include "runtime/editor/components_manager.h"
#include "runtime/editor/components/component_tracks.h"
#include "runtime/editor/components/utils/orderlist_utils.h"
#include "runtime/editor/datasources/datasource_orderlist.h"
#include "runtime/editor/datasources/datasource_sequence.h"
#include "runtime/editor/dialog/dialog_hex_value_input.h"
#include "runtime/editor/undo/undo.h"
#include "runtime/editor/undo/undo_componentdata/undo_componentdata_tracks.h"
#include "runtime/editor/utilities/editor_utils.h"
#include "runtime/editor/utilities/datasource_utils.h"
#include "runtime/editor/debug/debug_singleton.h"
#include "runtime/editor/datacopy/copypaste.h"
#include "runtime/editor/datacopy/datacopy_sequence.h"
#include "foundation/input/mouse.h"
#include "foundation/input/keyboard.h"
#include "foundation/input/keyboard_utils.h"
#include "foundation/graphics/textfield.h"
#include "utils/keyhook.h"
#include "utils/keyhookstore.h"
#include "utils/usercolors.h"

#include <string>
#include "foundation/base/assert.h"
#include "utils/logging.h"

using namespace Foundation;
using namespace Utility;

namespace Editor
{
	std::string ComponentTrack::ms_NotesSharp[12] =
	{
		"C-",
		"C#",
		"D-",
		"D#",
		"E-",
		"F-",
		"F#",
		"G-",
		"G#",
		"A-",
		"A#",
		"B-"
	};

	std::string ComponentTrack::ms_NotesFlat[12] =
	{
		"C-",
		"Db",
		"D-",
		"Eb",
		"E-",
		"F-",
		"Gb",
		"G-",
		"Ab",
		"A-",
		"Bb",
		"B-"
	};


	//----------------------------------------------------------------------------------------------------------------------------------------

	ComponentTrack::EventPosDetails::EventPosDetails()
		: m_OrderListIndex(0)
		, m_SequenceIndex(0)
	{
	}

	void ComponentTrack::EventPosDetails::Set(unsigned int inOrderListIndex, unsigned int inSequenceIndex)
	{
		m_OrderListIndex = inOrderListIndex;
		m_SequenceIndex = inSequenceIndex;
	}

	unsigned int ComponentTrack::EventPosDetails::OrderListIndex() const 
	{ 
		return m_OrderListIndex; 
	}
	
	unsigned int ComponentTrack::EventPosDetails::SequenceIndex() const 
	{ 
		return m_SequenceIndex; 
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	ComponentTrack::ComponentTrack(
		int inID,
		int inGroupID,
		Undo* inUndo,
		std::shared_ptr<DataSourceOrderList> inDataSourceOrderList,
		const std::vector<std::shared_ptr<DataSourceSequence>>& inDataSourceSequenceList,
		Foundation::TextField* inTextField,
		const EditState& inEditState,
		const Utility::KeyHookStore& inKeyHookStore,
		const AuxilaryDataCollection& inAuxilaryDataCollection,
		std::function<void(bool, int, int)> inStatusReportFunction,
		std::function<unsigned char()> inGetFirstFreeSequenceIndexFunction,
		std::function<unsigned char()> inGetFirstEmptySequenceIndexFunction,
		int inX,
		int inY,
		int inHeight
	)
		: ComponentBase(inID, inGroupID, inUndo, inTextField, inX, inY, 15, inHeight)
		, m_EditState(inEditState)
		, m_AuxilaryDataPlayMarkers(inAuxilaryDataCollection)
		, m_StatusReportFunction(inStatusReportFunction)
		, m_GetFirstFreeSequenceIndexFunction(inGetFirstFreeSequenceIndexFunction)
		, m_GetFirstEmptySequenceIndexFunction(inGetFirstEmptySequenceIndexFunction)
		, m_DataSourceOrderList(inDataSourceOrderList)
		, m_DataSourceSequenceList(inDataSourceSequenceList)
		, m_CursorPos(0)
		, m_EventPos(0xffffffff)
		, m_MaxEventPos(0)
		, m_FirstValidOrderListIndex(0)
		, m_FirstValidSequenceIndex(0)
		, m_HasFirstValid(false)
		, m_FocusModeOrderList(false)
		, m_TakingOrderListInput(false)
		, m_SequenceDataHasChanged(false)
		, m_HasDataChangeOrderList(false)
		, m_LocalDataChange(false)
		, m_IsMuted(false)
		, m_IsMarkingArea(false)
		, m_MarkingFromEventPos(0)
		, m_MarkingToEventPos(0)
	{
		UpdateMaxEventPos();

		m_FocusRow = ComponentTrackUtils::CalculateFocusRow(m_Dimensions.m_Height >> 1, m_Dimensions.m_Height);

		ConfigureKeyHooks(inKeyHookStore);
	}

	ComponentTrack::~ComponentTrack()
	{
		m_OrderListChangedEvent.Clear();
		m_SequenceChangedEvent.Clear();
		m_SequenceSplitEvent.Clear();
		m_OrderListIndexChangedEvent.Clear();
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentTrack::SetPosition(const Foundation::Point& inPosition)
	{
		m_Position = inPosition;
		m_Rect = { m_Position, m_Dimensions };
	}

	void ComponentTrack::SetHeight(int inHeight)
	{
		m_Dimensions.m_Height = inHeight;
		m_Rect = { m_Position, m_Dimensions };

		m_FocusRow = ComponentTrackUtils::CalculateFocusRow(m_Dimensions.m_Height >> 1, m_Dimensions.m_Height);
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentTrack::SetHasControl(GetControlType inGetControlType, CursorControl& inCursorControl)
	{
		ComponentBase::SetHasControl(inGetControlType, inCursorControl);

		if (m_EventPos > m_MaxEventPos)
			SetEventPosition(m_MaxEventPos);
		else
		{
			UpdateOrderListStatusReport();
			UpdateSequenceStatusReport();
		}

		{
			const unsigned int order_list_index = m_EventPosDetails.OrderListIndex();
			const auto& order_list_entry = (*m_DataSourceOrderList)[order_list_index];
			if (order_list_entry.m_Transposition >= 0xfe)
				m_OrderListIndexChangedEvent.Execute(m_HasControl, order_list_index, 0xff);
			else
				m_OrderListIndexChangedEvent.Execute(m_HasControl, order_list_index, order_list_entry.m_SequenceIndex);
		}
	}


	void ComponentTrack::ClearHasControl(CursorControl& inCursorControl)
	{
		ComponentBase::ClearHasControl(inCursorControl);

		if (m_IsMarkingArea)
			DoCancelMarking();

		{
			const unsigned int order_list_index = m_EventPosDetails.OrderListIndex();
			const auto& order_list_entry = (*m_DataSourceOrderList)[order_list_index];
			if (order_list_entry.m_Transposition >= 0xfe)
				m_OrderListIndexChangedEvent.Execute(m_HasControl, order_list_index, 0xff);
			else
				m_OrderListIndexChangedEvent.Execute(m_HasControl, order_list_index, order_list_entry.m_SequenceIndex);
		}
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	bool ComponentTrack::ConsumeInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		bool consume = false;
		int new_event_pos = m_EventPos;

		if (m_HasControl)
		{
			if (!m_EditState.IsPreventingSequenceEdit())
			{
				for (const auto& key_event : inKeyboard.GetKeyEventList())
				{
					if (Utility::ConsumeInputKeyHooks(key_event, inKeyboard.GetModiferMask(), m_KeyHooks, KeyHookContext({ inComponentsManager, new_event_pos })))
						consume = true;

					if (m_FocusModeOrderList)
					{
						if (inKeyboard.GetModiferMask() == 0)
						{
							char value_key = KeyboardUtils::FilterHexDigit(key_event);

							if (value_key != 0)
							{
								ApplyOrderListValueKey(value_key);
								consume = true;
							}
						}
					}
					else
					{
						if (inKeyboard.GetModiferMask() == 0)
						{
							if (IsCursorAtSequenceNote())
							{
								int note_value = EditorUtils::GetNoteValue(key_event, m_EditState.GetOctave());

								if (note_value != -1)
								{
									new_event_pos = ApplySequenceNoteValue(note_value);
									m_SequenceDataHasChanged = true;
									consume = true;
								}
							}
							else
							{
								char value_key = KeyboardUtils::FilterHexDigit(key_event);

								if (value_key != 0)
								{
									new_event_pos = ApplySequenceValueKey(value_key);
									m_SequenceDataHasChanged = true;
									consume = true;
								}
							}
						}
					}
				}

				if (new_event_pos != m_EventPos)
				{
					// If by any means the event position changed, we need to cancel the order list input.
					CancelOrderListInputValue();

					// Set the new event position
					SetEventPosition(new_event_pos);
				}
			}

			ApplyCursorPosition(inCursorControl);
		}

		if (m_SequenceDataHasChanged)
		{
			m_SequenceChangedEvent.Execute();
			m_SequenceDataHasChanged = false;
		}

		if (m_HasDataChangeOrderList)
		{
			m_OrderListChangedEvent.Execute();
		}

		return consume;
	}


	bool ComponentTrack::ConsumeInput(const Foundation::Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		if (inMouse.IsButtonPressed(Foundation::Mouse::Button::Left))
		{
			Foundation::Point screen_position = inMouse.GetPosition();

			if (ContainsPosition(screen_position))
			{
				Point cell_position = GetCellPosition(screen_position);

				const int event_pos_at_cursor = (cell_position.m_Y - m_Position.m_Y) + m_TopEventPos;
				const bool event_pos_at_cursor_outside_range = event_pos_at_cursor < 0 || event_pos_at_cursor >= m_MaxEventPos;

				if (!event_pos_at_cursor_outside_range)
				{
					int sequence_cursor_position_x = GetCursorPositionXFromSequenceCellX(cell_position.m_X);
					if (sequence_cursor_position_x >= 0)
					{
						if (m_FocusModeOrderList)
							SetFocusModeOrderList(false);

						m_CursorPos = sequence_cursor_position_x;

						ApplyCursorPosition(inCursorControl);
					}
					else if (IsEventPosStartOfSequence(event_pos_at_cursor))
					{
						if (m_EditState.IsPreventingSequenceEdit())
							return false;

						int orderlist_cursor_position_x = GetCursorPositionXFromOrderListCellX(cell_position.m_X);

						if (orderlist_cursor_position_x >= 0)
						{
							if (!m_FocusModeOrderList)
								SetFocusModeOrderList(true);

							m_CursorPos = orderlist_cursor_position_x;
							ApplyCursorPosition(inCursorControl);
						}
					}
				}

				if (!m_EditState.IsPreventingSequenceEdit())
					SetEventPosition(std::min(m_MaxEventPos, std::max(0, event_pos_at_cursor)));

				return true;
			}
		}

		return false;
	}


	bool ComponentTrack::ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse)
	{
		return false;
	}


	void ComponentTrack::Refresh(const DisplayState& inDisplayState)
	{
		if (m_RequireRefresh)
		{
			m_RequireRefresh = false;

			const Color background_color = ToColor(UserColor::TrackBackground);
			const Color background_color_muted = ToColor(UserColor::TrackBackgroundMuted);
			const Color focus_line_background_color = ToColor(UserColor::TrackBackgroundFocusLine);
			const Color focus_line_background_color_muted = ToColor(UserColor::TrackBackgroundMutedFocusLine);

			Rect focus_line_rect = m_Rect;

			focus_line_rect.m_Position.m_Y += m_Rect.m_Dimensions.m_Height >> 1;
			focus_line_rect.m_Dimensions.m_Height = 1;

			m_TextField->ColorAreaBackground(m_IsMuted ? background_color_muted : background_color, m_Rect);
			m_TextField->ColorAreaBackground(m_IsMuted ? focus_line_background_color_muted : focus_line_background_color, focus_line_rect);
			m_TextField->ClearText(m_Rect);

			if (m_HasFirstValid)
			{
				const bool is_uppercase = inDisplayState.IsHexUppercase();

				const int top_event = m_TopEventPos;
				const int bottom_event = m_TopEventPos + m_Dimensions.m_Height;
				const unsigned int loop_index = static_cast<unsigned int>(m_DataSourceOrderList->GetLoopIndex());

				int current_y = m_Position.m_Y;
				int current_event = top_event;

				// Draw marking
				if (m_IsMarkingArea)
				{
					int top = std::min(m_MarkingFromEventPos, m_MarkingToEventPos);
					int bottom = std::max(m_MarkingFromEventPos, m_MarkingToEventPos);

					int marking_top_screen_position = top - top_event;
					int marking_bottom_screen_position = (bottom - top_event) + 1;

					const bool out_of_screen = (marking_top_screen_position < 0 && marking_bottom_screen_position < 0)
						|| (marking_top_screen_position >= m_Dimensions.m_Height && marking_bottom_screen_position >= m_Dimensions.m_Height);

					if (!out_of_screen)
					{
						marking_top_screen_position = marking_top_screen_position < 0 ? 0 : marking_top_screen_position;
						marking_bottom_screen_position = marking_bottom_screen_position >= m_Dimensions.m_Height ? m_Dimensions.m_Height : marking_bottom_screen_position;

						Rect marking_rect = m_Rect;

						marking_rect.m_Position.m_Y += marking_top_screen_position;
						marking_rect.m_Dimensions.m_Height = marking_bottom_screen_position - marking_top_screen_position;
						marking_rect.m_Position.m_X += 5;
						marking_rect.m_Dimensions.m_Width -= 6;

						const Color background_color_marking = Color::DarkerBlue;
						const Color background_color_marking_muted = Color::DarkerGrey;

						m_TextField->ColorAreaBackground(m_IsMuted ? background_color_marking_muted : background_color_marking, marking_rect);
					}
				}

				// Figure blank height
				if (top_event < 0)
				{
					int blank_height = -top_event;
					current_y += blank_height;
					current_event += blank_height;
				}

				unsigned char current_instrument = 0xff;

				unsigned int orderlist_index = m_FirstValidOrderListIndex;
				unsigned int sequence_index = m_FirstValidSequenceIndex;

				{
					DataSourceOrderList::Entry& order_list_entry = (*m_DataSourceOrderList)[orderlist_index];
					const std::shared_ptr<DataSourceSequence>& sequence = m_DataSourceSequenceList[order_list_entry.m_SequenceIndex];

					for (unsigned int i = 0; i < sequence_index; ++i)
					{
						const DataSourceSequence::Event& event = (*sequence)[i];
						if ((event.m_Instrument & 0xe0) == 0xa0)
							current_instrument = event.m_Instrument & 0x1f;
					}
				}

				// Draw sequence lines
				SequenceColors sequence_colors(
				{
					ToColor(UserColor::SequenceError),
					ToColor(UserColor::SequenceInstrumentEmpty),
					ToColor(UserColor::SequenceInstrumentEmptyFocusLine),
					ToColor(UserColor::SequenceInstrumentTie),
					ToColor(UserColor::SequenceInstrumentTieFocusLine),
					ToColor(UserColor::SequenceInstrumentValue),
					ToColor(UserColor::SequenceInstrumentValueFocusLine),
					ToColor(UserColor::SequenceInstrumentValueSelectedHighlight),
					ToColor(UserColor::SequenceInstrumentValueSelectedHighlightFocusLine),
					ToColor(UserColor::SequenceCommandEmpty),
					ToColor(UserColor::SequenceCommandEmptyFocusLine),
					ToColor(UserColor::SequenceCommandValue),
					ToColor(UserColor::SequenceCommandValueFocusLine),
					ToColor(UserColor::SequenceCommandValueSelectedHighlight),
					ToColor(UserColor::SequenceCommandValueSelectedHighlightFocusLine),
					ToColor(UserColor::SequenceNoteEmpty),
					ToColor(UserColor::SequenceNoteEmptyFocusLine),
					ToColor(UserColor::SequenceNoteGateOn),
					ToColor(UserColor::SequenceNoteGateOnFocusLine),
					ToColor(UserColor::SequenceNoteValue),
					ToColor(UserColor::SequenceNoteValueFocusLine),
					ToColor(UserColor::SequenceNoteValueTied),
					ToColor(UserColor::SequenceNoteValueTiedFocusLine),
					ToColor(UserColor::SequenceNoteValueSelectedHighlight),
					ToColor(UserColor::SequenceNoteValueSelectedHighlightFocusLine)
				});

				const Color color_orderlist_end_loop = ToColor(UserColor::OrderlistEndLoop);
				const Color color_orderlist_end_stop = ToColor(UserColor::OrderlistEndStop);
				const Color color_orderlist_value = ToColor(UserColor::OrderlistValue);
				const Color color_orderlist_value_loop_marker = ToColor(UserColor::OrderlistValueLoopMarker);
				const Color color_orderlist_value_input = ToColor(UserColor::OrderlistValueInput);

				while (current_event < bottom_event)
				{
					DataSourceOrderList::Entry& order_list_entry = (*m_DataSourceOrderList)[orderlist_index];
					const std::shared_ptr<DataSourceSequence>& sequence = m_DataSourceSequenceList[order_list_entry.m_SequenceIndex];

					if (sequence_index == 0)
					{
						if (order_list_entry.m_Transposition == 0xff)
							m_TextField->Print(m_Position.m_X, current_y, color_orderlist_end_loop, "loop");
						else if(order_list_entry.m_Transposition == 0xfe)
							m_TextField->Print(m_Position.m_X, current_y, color_orderlist_end_stop, "stop");
						else
						{
							if (m_HasControl && m_TakingOrderListInput && orderlist_index == m_EventPosDetails.OrderListIndex())
							{
								m_TextField->PrintHexValue(m_Position.m_X, current_y, color_orderlist_value_input, is_uppercase, m_OrderListInputValue);
							}
							else
							{
								const bool is_looping_point = (orderlist_index == loop_index);
								const Color color = is_looping_point ? color_orderlist_value_loop_marker : color_orderlist_value;

								m_TextField->PrintHexValue(m_Position.m_X, current_y, color, is_uppercase, order_list_entry.m_Transposition);
								m_TextField->PrintHexValue(m_Position.m_X + 2, current_y, color, is_uppercase, order_list_entry.m_SequenceIndex);
							}
						}
					}

					if (current_event >= m_MaxEventPos)
						break;

					int transposition = static_cast<int>(order_list_entry.m_Transposition) - 0xa0;
					DrawSequenceLine(m_Position.m_X + 5, current_y, sequence_colors, is_uppercase, sequence, sequence_index, transposition, current_event == m_EventPos, current_instrument);

					++current_y;
					++current_event;
					++sequence_index;

					if (sequence_index >= sequence->GetLength())
					{
						++orderlist_index;
						sequence_index = 0;
					}
				}
			}
		}
	}


	void ComponentTrack::HandleDataChange()
	{
		if (m_HasDataChange)
		{
			if (m_HasDataChangeOrderList)
			{
				if (m_DataSourceOrderList->PushDataToSource())
					m_HasDataChangeOrderList = false;
			}

			if (!m_DataChangeSequenceIndexList.empty())
			{
				std::vector<unsigned char> skipped_data_pushes;
				for (unsigned char sequence_index : m_DataChangeSequenceIndexList)
				{
					const std::shared_ptr<DataSourceSequence>& sequence = m_DataSourceSequenceList[sequence_index];

					if (!sequence->PushDataToSource())
						skipped_data_pushes.push_back(sequence_index);
				}

				m_DataChangeSequenceIndexList = skipped_data_pushes;
			}

			m_HasDataChange = !m_DataChangeSequenceIndexList.empty();

			if (!m_HasDataChange || (!m_HasDataChangeOrderList && m_LocalDataChange))
				AddUndoRecentModificationStep(false);

			m_LocalDataChange = false;
		}
	}


	void ComponentTrack::PullDataFromSource(const bool inFromUndo)
	{
		m_DataSourceOrderList->PullDataFromSource();
		UpdateMaxEventPos();
	}


	void ComponentTrack::PullSequenceDataFromSource()
	{
		for (auto& sequence : m_DataSourceSequenceList)
			sequence->PullDataFromSource();
	}


	void ComponentTrack::ExecuteInsertDeleteRule(const DriverInfo::TableInsertDeleteRule& inRule, int inSourceTableID, int inIndexPre, int inIndexPost)
	{

	}


	void ComponentTrack::ExecuteAction(int inActionInput)
	{

	}



	void ComponentTrack::ForceRefresh()
	{
		UpdateMaxEventPos();
		m_RequireRefresh = true;
	}


	//--------------------------------------------------------------------------------------

	int ComponentTrack::GetCursorPosition() const
	{
		return m_CursorPos;
	}


	void ComponentTrack::SetCursorPosition(int inCursorPosition)
	{
		if (m_TakingOrderListInput)
			m_CursorPos = inCursorPosition < 0 ? 0 : (inCursorPosition > 3 ? 3 : inCursorPosition);
		else
			m_CursorPos = inCursorPosition < 0 ? 0 : (inCursorPosition > 4 ? 4 : inCursorPosition);
	}


	//--------------------------------------------------------------------------------------

	int ComponentTrack::GetMaxEventPosition() const
	{
		return m_MaxEventPos;
	}


	void ComponentTrack::SetEventPosition(int inEventPos)
	{
		FOUNDATION_ASSERT(inEventPos >= 0);

		m_EventPos = inEventPos;
		m_TopEventPos = inEventPos - m_FocusRow.m_RowsAbove;

		bool found_event_pos = false;
		bool found_top_event_pos = false;

		if (m_TopEventPos <= 0)
		{
			m_FirstValidOrderListIndex = 0;
			m_FirstValidSequenceIndex = 0;

			found_top_event_pos = true;
			m_HasFirstValid = true;
		}

		if (m_TopEventPos > m_MaxEventPos)
		{
			m_HasFirstValid = false;
		}
		else
		{
			int event_pos = 0;

			for (unsigned int i = 0; i < m_DataSourceOrderList->GetLength(); ++i)
			{
				unsigned char sequence_index = (*m_DataSourceOrderList)[i].m_SequenceIndex;
				const std::shared_ptr<DataSourceSequence>& sequence = m_DataSourceSequenceList[sequence_index];

				int next_event_pos = event_pos + sequence->GetLength();

				if (!found_top_event_pos && m_TopEventPos >= event_pos && m_TopEventPos < next_event_pos)
				{
					found_top_event_pos = true;

					m_FirstValidOrderListIndex = i;
					m_FirstValidSequenceIndex = m_TopEventPos - event_pos;
				}

				if (!found_event_pos && m_EventPos >= event_pos && m_EventPos < next_event_pos)
				{
					found_event_pos = true;
					SetEventPosDetails(i, m_EventPos - event_pos);

					break;
				}

				event_pos = next_event_pos;
			}

			m_HasFirstValid = true;
		}

		m_RequireRefresh = true;

		UpdateOrderListStatusReport();
		UpdateSequenceStatusReport();
	}

	int ComponentTrack::GetEventPosition() const
	{
		return m_EventPos;
	}

	int ComponentTrack::GetEventPositionAtTopOfCurrentSequence() const
	{
		int event_pos = 0;

		for (unsigned int i = 0; i < m_DataSourceOrderList->GetLength(); ++i)
		{
			const unsigned char transposition = (*m_DataSourceOrderList)[i].m_Transposition;
			const unsigned char sequence_index = (*m_DataSourceOrderList)[i].m_SequenceIndex;

			if (transposition == 0xff || transposition == 0xfe)
				break;

			const std::shared_ptr<DataSourceSequence>& sequence = m_DataSourceSequenceList[sequence_index];

			const int next_event_pos = event_pos + sequence->GetLength();

			if (m_EventPos >= event_pos && m_EventPos < next_event_pos)
				break;

			event_pos = next_event_pos;
		}

		return event_pos;
	}

	//--------------------------------------------------------------------------------------------------

	void ComponentTrack::SetFocusModeOrderList(bool inFocusModeOrderList)
	{
		if (m_IsMarkingArea)
			DoCancelMarking();

		m_FocusModeOrderList = inFocusModeOrderList;

		if (m_FocusModeOrderList)
			m_CursorPos = 0;
		else
		{
			m_CursorPos = 4;

			if(m_TakingOrderListInput)
				CancelOrderListInputValue();
		}

		// If the track has control, set the event pos to the beginning of the current sequence
		if (m_HasControl && m_FocusModeOrderList)
		{
			int event_pos = GetEventPositionAtTopOfCurrentSequence();
			SetEventPosition(event_pos);
		}

		UpdateOrderListStatusReport();
		UpdateSequenceStatusReport();
	}


	bool ComponentTrack::GetFocusModeOrderList() const
	{
		return m_FocusModeOrderList;
	}


	bool ComponentTrack::IsTakingOrderListInput() const
	{
		return m_TakingOrderListInput;
	}

	//--------------------------------------------------------------------------------------------------

	bool ComponentTrack::IsMuted() const
	{
		return m_IsMuted;
	}

	void ComponentTrack::SetMuted(bool inMuted)
	{
		m_IsMuted = inMuted;
		m_RequireRefresh = true;
	}

	//--------------------------------------------------------------------------------------------------

	void ComponentTrack::SetOrderlistLoopPoint()
	{
		if (m_EventPos < m_MaxEventPos)
		{
			EventPosDetails event_pos_details = GetEventPosDetails(m_EventPos);
			if (event_pos_details.OrderListIndex() >= 0)
			{
				FOUNDATION_ASSERT(event_pos_details.OrderListIndex() < 0x100);

				m_DataSourceOrderList->SetLoopIndex(static_cast<unsigned char>(event_pos_details.OrderListIndex()));

				OnOrderListChanged();

				m_HasDataChangeOrderList = true;
				m_HasDataChange = true;
				m_RequireRefresh = true;
			}
		}
	}


	int ComponentTrack::GetLoopEventPosition() const
	{
		const int loop_index = m_DataSourceOrderList->GetLoopIndex();

		if (loop_index == 0)
			return 0;

		int event_position = 0;
		for (int i = 0; i < loop_index; ++i)
		{
			const auto& entry = (*m_DataSourceOrderList)[i];
			FOUNDATION_ASSERT(entry.m_Transposition < 0xfe);
			const int sequence_length = m_DataSourceSequenceList[entry.m_SequenceIndex]->GetLength();
			event_position += sequence_length;
		}

		return event_position;
	}


	//--------------------------------------------------------------------------------------------------

	bool ComponentTrack::ConsumeHasInputCausedSequenceDataChange()
	{
		const bool return_value = m_SequenceDataHasChanged;
		m_SequenceDataHasChanged = false;

		return return_value;
	}

	//--------------------------------------------------------------------------------------------------

	bool ComponentTrack::ComputePlaybackStateFromEventPosition(int inEventPos, std::vector<IDriverArchitecture::PlayMarkerInfo>& outOrderListIndices) const
	{
		EventPosDetails event_pos_details = GetEventPosDetails(inEventPos);

		// Find position in packed order list
		DataSourceOrderList::PackedDataEventPosition order_list_event_position = m_DataSourceOrderList->GetIndexInPackedData(event_pos_details.OrderListIndex());

		if (order_list_event_position.m_IsValid)
		{
			// Find index in packed sequence data and remaining ticks to next event in packed sequence data
			const DataSourceOrderList::Entry& order_list_entry = (*m_DataSourceOrderList)[event_pos_details.OrderListIndex()];

			if (order_list_entry.m_Transposition < 0xfe)
			{
				DataSourceSequence::PackedDataEventPosition event_position = m_DataSourceSequenceList[order_list_entry.m_SequenceIndex]->GetEventPositionInPackedData(event_pos_details.SequenceIndex());

				if (event_position.m_IsValid)
				{
					outOrderListIndices.push_back(
						{
							static_cast<unsigned char>(order_list_event_position.m_Index),
							order_list_event_position.m_Sequence,
							order_list_event_position.m_CurrentTranspose,
							event_position.m_Index,
							event_position.m_CurrentDurationValue,
							event_position.m_RemainingTicks,
							event_position.m_NextInstrumentAddress,
							!event_position.m_NextIsEndOfSequence
						} );

					return true;
				}
			}
		}

		return false;
	}


	ComponentTrack::OrderListIndexChangedEvent& ComponentTrack::GetOrderListIndexChangedEvent()
	{
		return m_OrderListIndexChangedEvent;
	}

	ComponentTrack::SequenceSplitEvent& ComponentTrack::GetSequenceSplitEvent()
	{
		return m_SequenceSplitEvent;
	}

	ComponentTrack::SequenceChangedEvent& ComponentTrack::GetSequenceChangedEvent()
	{
		return m_SequenceChangedEvent;
	}

	ComponentTrack::OrderListChangedEvent& ComponentTrack::GetOrderListChangedEvent()
	{
		return m_OrderListChangedEvent;
	}

	ComponentTrack::ComputeMaxEventPosEvent& ComponentTrack::GetComputeMaxEventPosEvent()
	{
		return m_ComputedMaxEventPosEvent;
	}

	//--------------------------------------------------------------------------------------------------
	// Undo handlers
	//--------------------------------------------------------------------------------------------------

	void ComponentTrack::SetUndoHandlers(std::function<void(UndoComponentDataTableTracks&)> inAddUndoStepHandler, std::function<void(const UndoComponentDataTableTracks&, CursorControl&)> inOnUndoHandler)
	{
		m_AddUndoStepHandler = inAddUndoStepHandler;
		m_OnUndoHandler = inOnUndoHandler;
	}


	//--------------------------------------------------------------------------------------------------
	// Order list input
	//--------------------------------------------------------------------------------------------------

	bool ComponentTrack::BeginOrderListInput()
	{
		FOUNDATION_ASSERT(m_DataSourceOrderList != nullptr);
		FOUNDATION_ASSERT(m_DataSourceSequenceList.size() > 0);
		FOUNDATION_ASSERT(!m_TakingOrderListInput);

		const DataSourceOrderList::Entry orderlist_entry = (*m_DataSourceOrderList)[m_EventPosDetails.OrderListIndex()];

		if (orderlist_entry.m_Transposition < 0xfe)
		{
			m_OrderListInputValue = (static_cast<unsigned short>(orderlist_entry.m_Transposition) << 8) | orderlist_entry.m_SequenceIndex;
			m_TakingOrderListInput = true;

			return true;
		}

		return false;
	}


	void ComponentTrack::ApplyOrderListValueKey(char inValueKey)
	{
		if (m_TakingOrderListInput || BeginOrderListInput())
		{
			unsigned char value = EditorUtils::ConvertSingleCharHexValueToValue(inValueKey);

			switch (m_CursorPos)
			{
			case 0:
				value = value < 0x08 ? 0x08 : (value > 0x0b ? 0x0b : value);
				break;
			case 2:
				value = value > 0x07 ? 0x07 : value;
				break;
			}

			int shift = 4 * (3 - m_CursorPos);

			m_OrderListInputValue &= ~(0x0f << shift);
			m_OrderListInputValue |= value << shift;

			DoKeyRight(false);

			m_RequireRefresh = true;
		}
	}


	void ComponentTrack::ApplyOrderListInputValue()
	{
		FOUNDATION_ASSERT(m_DataSourceOrderList != nullptr);
		FOUNDATION_ASSERT(m_DataSourceSequenceList.size() > 0);
		FOUNDATION_ASSERT(m_TakingOrderListInput);

		AddUndoStep();

		DataSourceOrderList::Entry& orderlist_entry = (*m_DataSourceOrderList)[m_EventPosDetails.OrderListIndex()];

		orderlist_entry.m_Transposition = static_cast<unsigned char>(m_OrderListInputValue >> 8);
		orderlist_entry.m_SequenceIndex = static_cast<unsigned char>(m_OrderListInputValue & 0xff);

		m_TakingOrderListInput = false;

		OnOrderListChanged();
	}


	void ComponentTrack::CancelOrderListInputValue()
	{
		if (m_TakingOrderListInput)
		{
			m_TakingOrderListInput = false;
			m_RequireRefresh = true;
		}
	}


	void ComponentTrack::HandleOrderListUpdateAfterSequenceSplit(unsigned char inSequenceIndex, unsigned char inAddSequenceIndex)
	{
		const int length = m_DataSourceOrderList->GetLength();

		for (int i = length - 1; i >= 0; --i)
		{
			const auto& entry = (*m_DataSourceOrderList)[i];
			if (entry.m_Transposition <= 0xfe && entry.m_SequenceIndex == inSequenceIndex)
			{
				auto new_entry = entry;
				new_entry.m_SequenceIndex = inAddSequenceIndex;

				OrderListInsert(m_DataSourceOrderList, i + 1, new_entry);
			}
		}

		OnOrderListChanged();
	}

	//--------------------------------------------------------------------------------------------------
	// Draw sequence entry
	//--------------------------------------------------------------------------------------------------

	void ComponentTrack::DrawSequenceLine(int inX, int inY, const SequenceColors& inColors, const bool inIsHexUppercase, const std::shared_ptr<DataSourceSequence>& inSequence, unsigned int inSequenceIndex, int inTransposition, const bool inIsFocusLine, unsigned char& ioCurrentInstrument)
	{
		const bool in_error_state = inSequence->IsInErrorState();

		const DataSourceSequence::Event& event = (*inSequence)[inSequenceIndex];

		if ((event.m_Instrument & 0xe0) == 0xa0)
			ioCurrentInstrument = event.m_Instrument & 0x1f;

		const bool is_selected_instrument = m_EditState.IsSelectedInstrument(ioCurrentInstrument);
		const bool is_selected_command = m_EditState.IsSelectedCommand(event.m_Command & 0x3f);

		const Point instrument_pos = Point(inX + 0, inY);
		const Point command_pos = Point(inX + 3, inY);
		const Point note_pos = Point(inX + 6, inY);

		// Instrument column
		{
			const Color empty = in_error_state ? inColors.m_ErrorState :
				(inIsFocusLine ? inColors.m_InstrumentEmptyFocusLine : inColors.m_InstrumentEmpty);
			const Color tie_note = in_error_state ? inColors.m_ErrorState :
				(inIsFocusLine ? inColors.m_InstrumentTieNoteFocusLine : inColors.m_InstrumentTieNote);
			const Color value = in_error_state ? inColors.m_ErrorState :
				(is_selected_instrument ?
					(inIsFocusLine ? inColors.m_InstrumentValueSelectedHighlightFocusLine : inColors.m_InstrumentValueSelectedHighlight) :
					(inIsFocusLine ? inColors.m_InstrumentValueFocusLine : inColors.m_InstrumentValue));

			if (event.m_Instrument == 0x80)
				m_TextField->Print(instrument_pos, empty, "--");
			else if (event.m_Instrument == 0x90)
				m_TextField->Print(instrument_pos, tie_note, "**");
			else if ((event.m_Instrument & 0xe0) == 0xa0)
				m_TextField->Print(instrument_pos, value, ComponentTrack::ToHexValueString(event.m_Instrument & 0x1f, inIsHexUppercase));
			else
				m_TextField->Print(instrument_pos, inColors.m_ErrorState, "??");
		}

		// Command column
		{
			const Color empty = in_error_state ? inColors.m_ErrorState :
				(inIsFocusLine ? inColors.m_CommandEmptyFocusLine : inColors.m_CommandEmpty);
			const Color value = in_error_state ? inColors.m_ErrorState :
				(is_selected_command ?
					(inIsFocusLine ? inColors.m_CommandValueSelectedHighlightFocusLine : inColors.m_CommandValueSelectedHighlight) :
					(inIsFocusLine ? inColors.m_CommandValueFocusLine : inColors.m_CommandValue));

			if (event.m_Command == 0x80)
				m_TextField->Print(command_pos, empty, "--");
			else if ((event.m_Command & 0xc0) == 0xc0)
				m_TextField->Print(command_pos, value, ComponentTrack::ToHexValueString(event.m_Command & 0x3f, inIsHexUppercase));
			else
				m_TextField->Print(command_pos, inColors.m_ErrorState, "??");
		}

		// Note column
		{
			const bool is_tie_note = event.m_Instrument == 0x90;

			const Color empty = in_error_state ? inColors.m_ErrorState :
				(inIsFocusLine ? inColors.m_NoteEmptyFocusLine : inColors.m_NoteEmpty);
			const Color gate_on = in_error_state ? inColors.m_ErrorState :
				(inIsFocusLine ? inColors.m_NoteGateOnFocusLine : inColors.m_NoteGateOn);
			const Color value = in_error_state ? inColors.m_ErrorState :
				(is_tie_note ?
					(inIsFocusLine ? inColors.m_NoteValueTiedFocusLine : inColors.m_NoteValueTied) :
					(is_selected_instrument ?
						(inIsFocusLine ? inColors.m_NoteValueSelectedHighlightFocusLine : inColors.m_NoteValueSelectedHighlight) :
						(inIsFocusLine ? inColors.m_NoteValueFocusLine : inColors.m_NoteValue)));


			int transposed_note = static_cast<int>(event.m_Note) + inTransposition;

			if (event.m_Note == 0)
				m_TextField->Print(note_pos, empty, "---");
			else if (event.m_Note == 0x7e)
				m_TextField->Print(note_pos, gate_on, "+++");
			else if (transposed_note >= 0x01 && transposed_note < 0x60)
			{
				unsigned char note = static_cast<unsigned char>(transposed_note % 12);
				unsigned char octave = static_cast<unsigned char>(transposed_note / 12);

				const AuxilaryDataEditingPreferences::NotationMode notation_mode = m_AuxilaryDataPlayMarkers.GetEditingPreferences().GetNotationMode();

				const std::string& note_string = (notation_mode == AuxilaryDataEditingPreferences::NotationMode::Sharp)
					? ComponentTrack::ms_NotesSharp[note]
					: ComponentTrack::ms_NotesFlat[note];

				m_TextField->Print(note_pos, value, note_string + std::to_string(octave));
			}
			else
				m_TextField->Print(note_pos, inColors.m_ErrorState, "???");
		}
	}


	bool ComponentTrack::IsCursorAtSequenceInstrument() const
	{
		return (!m_FocusModeOrderList && m_CursorPos < 2);
	}


	bool ComponentTrack::IsCursorAtSequenceCommand() const
	{
		return (!m_FocusModeOrderList && m_CursorPos >= 2 && m_CursorPos < 4);
	}


	bool ComponentTrack::IsCursorAtSequenceNote() const
	{
		return (!m_FocusModeOrderList && m_CursorPos >= 4);
	}


	unsigned int ComponentTrack::GetSequenceSize() const
	{
		DataSourceOrderList::Entry order_list_entry = (*m_DataSourceOrderList)[m_EventPosDetails.OrderListIndex()];

		if (order_list_entry.m_Transposition < 0xfe)
		{
			const std::shared_ptr<DataSourceSequence>& sequence_data_source = m_DataSourceSequenceList[order_list_entry.m_SequenceIndex];
			return sequence_data_source->GetLength();
		}

		return 0;
	}


	int ComponentTrack::ApplySequenceValueKey(char inValueKey)
	{
		FOUNDATION_ASSERT(!IsCursorAtSequenceNote());

		DataSourceOrderList::Entry order_list_entry = (*m_DataSourceOrderList)[m_EventPosDetails.OrderListIndex()];

		if (order_list_entry.m_Transposition < 0xfe)
		{
			const std::shared_ptr<DataSourceSequence>& sequence_data_source = m_DataSourceSequenceList[order_list_entry.m_SequenceIndex];
			DataSourceSequence::Event& event = (*sequence_data_source)[m_EventPosDetails.SequenceIndex()];

			unsigned char value = EditorUtils::ConvertSingleCharHexValueToValue(inValueKey);
			bool change_made = false;

			if (IsCursorAtSequenceInstrument())
			{
				if(!sequence_data_source->IsInErrorState())
					AddUndoStep();

				unsigned char instrument = event.m_Instrument & 0x1f;

				if ((m_CursorPos & 1) == 0)
				{
					if (value > 1)
						return m_EventPos;

					instrument = (instrument & 0x0f) | (value << 4);
				}
				else
					instrument = (instrument & 0xf0) | value;

				instrument |= 0xa0;

				if (event.m_Instrument != instrument)
				{
					event.m_Instrument = instrument;
					change_made = true;
				}
			}
			else if (IsCursorAtSequenceCommand())
			{
				if (!sequence_data_source->IsInErrorState())
					AddUndoStep();

				unsigned char command = event.m_Command & 0x3f;

				if ((m_CursorPos & 1) == 0)
				{
					if (value > 3)
						return m_EventPos;

					command = (command & 0x0f) | (value << 4);
				}
				else
					command = (command & 0xf0) | value;

				command |= 0xc0;

				if (event.m_Command != command)
				{
					event.m_Command = command | 0xc0;
					change_made = true;
				}
			}

			if (change_made)
			{
				DoKeyRight(false);

				OnSequenceChanged(order_list_entry.m_SequenceIndex);
			}
		}

		return m_EventPos;
	}


	int ComponentTrack::ApplySequenceNoteValue(int inNoteValue)
	{
		FOUNDATION_ASSERT(IsCursorAtSequenceNote());

		DataSourceOrderList::Entry order_list_entry = (*m_DataSourceOrderList)[m_EventPosDetails.OrderListIndex()];

		if (order_list_entry.m_Transposition < 0xfe)
		{
			const std::shared_ptr<DataSourceSequence>& sequence_data_source = m_DataSourceSequenceList[order_list_entry.m_SequenceIndex];
			DataSourceSequence::Event& event = (*sequence_data_source)[m_EventPosDetails.SequenceIndex()];

			const int transpose = static_cast<int>(order_list_entry.m_Transposition) - 0xa0;
			int note_value = inNoteValue - transpose;

			if (note_value > 0 && note_value < 0x60)
			{
				if (!sequence_data_source->IsInErrorState())
					AddUndoStep();

				if (event.m_Note != note_value)
				{
					event.m_Note = static_cast<unsigned char>(note_value);
					OnSequenceChanged(order_list_entry.m_SequenceIndex);
				}

				UpdateSequenceStatusReport();

				return DoKeyDown();
			}
		}

		return m_EventPos;
	}


	int ComponentTrack::ApplySequenceHoldNoteValue()
	{
		DataSourceOrderList::Entry order_list_entry = (*m_DataSourceOrderList)[m_EventPosDetails.OrderListIndex()];

		if (order_list_entry.m_Transposition < 0xfe)
		{
			const std::shared_ptr<DataSourceSequence>& sequence_data_source = m_DataSourceSequenceList[order_list_entry.m_SequenceIndex];
			DataSourceSequence::Event& event = (*sequence_data_source)[m_EventPosDetails.SequenceIndex()];

			if (!sequence_data_source->IsInErrorState())
				AddUndoStep();

			if (event.m_Note != 0x7e)
			{
				event.m_Note = 0x7e;
				OnSequenceChanged(order_list_entry.m_SequenceIndex);
			}

			return DoKeyDown();
		}

		return m_EventPos;
	}


	bool ComponentTrack::ApplyTranspose(char inDelta)
	{
		DataSourceOrderList::Entry order_list_entry = (*m_DataSourceOrderList)[m_EventPosDetails.OrderListIndex()];

		if (order_list_entry.m_Transposition < 0xfe)
		{
			const std::shared_ptr<DataSourceSequence>& sequence_data_source = m_DataSourceSequenceList[order_list_entry.m_SequenceIndex];
			const int length = static_cast<int>(sequence_data_source->GetLength());

			// Validate
			for (int i = 0; i < length; ++i)
			{
				DataSourceSequence::Event& event = (*sequence_data_source)[i];

				if (event.m_Note > 0 && event.m_Note <= 0x60)
				{
					char transposed_note = static_cast<char>(event.m_Note) + inDelta;
					if (transposed_note > 0x60 || transposed_note <= 0)
						return false;
				}
			}

			// Do transpose
			if (!sequence_data_source->IsInErrorState())
				AddUndoStep();

			for (int i = 0; i < length; ++i)
			{
				DataSourceSequence::Event& event = (*sequence_data_source)[i];

				if (event.m_Note > 0 && event.m_Note <= 0x60)
				{
					char transposed_note = static_cast<char>(event.m_Note) + inDelta;
					event.m_Note = static_cast<unsigned char>(transposed_note);
				}
			}

			OnSequenceChanged(order_list_entry.m_SequenceIndex);


			return true;
		}

		return false;
	}


	void ComponentTrack::ToggleSequenceHoldNoteUntilEvent(bool inDown)
	{
		DataSourceOrderList::Entry order_list_entry = (*m_DataSourceOrderList)[m_EventPosDetails.OrderListIndex()];

		if (order_list_entry.m_Transposition < 0xfe)
		{
			const std::shared_ptr<DataSourceSequence>& sequence_data_source = m_DataSourceSequenceList[order_list_entry.m_SequenceIndex];

			if (!sequence_data_source->IsInErrorState())
				AddUndoStep();

			int event_pos_index = m_EventPosDetails.SequenceIndex();

			unsigned char note = (*sequence_data_source)[event_pos_index].m_Note;

			if (note != 00 && note != 0x7e)
			{
				event_pos_index += inDown ? 1 : -1;

				if (event_pos_index < 0 || event_pos_index >= static_cast<int>(sequence_data_source->GetLength()))
					return;

				note = (*sequence_data_source)[event_pos_index].m_Note;
			}
			const bool set_gate = (note == 0x00);

			bool changed = false;

			int i = event_pos_index;

			while (i >= 0 && i < static_cast<int>(sequence_data_source->GetLength()))
			{
				DataSourceSequence::Event& event = (*sequence_data_source)[i];

				if (set_gate)
				{
					if (event.m_Note != 0x00)
						break;

					event.m_Note = 0x7e;
					changed = true;
				}
				else
				{
					if (event.m_Note != 0x7e)
						break;

					event.m_Note = 0x00;
					changed = true;
				}

				i += inDown ? 1 : -1;
			}

			if(changed)
				OnSequenceChanged(order_list_entry.m_SequenceIndex);
		}
	}


	void ComponentTrack::ToggleSequenceTieNote()
	{
		DataSourceOrderList::Entry order_list_entry = (*m_DataSourceOrderList)[m_EventPosDetails.OrderListIndex()];

		if (order_list_entry.m_Transposition < 0xfe)
		{
			const std::shared_ptr<DataSourceSequence>& sequence_data_source = m_DataSourceSequenceList[order_list_entry.m_SequenceIndex];
			DataSourceSequence::Event& event = (*sequence_data_source)[m_EventPosDetails.SequenceIndex()];

			if (!sequence_data_source->IsInErrorState())
				AddUndoStep();

			if (event.m_Instrument != 0x90)
				event.m_Instrument = 0x90;
			else
				event.m_Instrument = 0x80;

			OnSequenceChanged(order_list_entry.m_SequenceIndex);
		}
	}


	int ComponentTrack::EraseSequenceLine(bool inValueOnlyAtCursor)
	{
		DataSourceOrderList::Entry order_list_entry = (*m_DataSourceOrderList)[m_EventPosDetails.OrderListIndex()];

		if (order_list_entry.m_Transposition < 0xfe)
		{
			const std::shared_ptr<DataSourceSequence>& sequence_data_source = m_DataSourceSequenceList[order_list_entry.m_SequenceIndex];
			DataSourceSequence::Event& event = (*sequence_data_source)[m_EventPosDetails.SequenceIndex()];

			if (!sequence_data_source->IsInErrorState())
				AddUndoStep();

			bool change_made = false;

			if (!inValueOnlyAtCursor || IsCursorAtSequenceInstrument())
			{
				if (event.m_Instrument != 0x80)
				{
					change_made |= true;
					event.m_Instrument = 0x80;
				}
			}

			if (!inValueOnlyAtCursor || IsCursorAtSequenceCommand())
			{
				if (event.m_Command != 0x80)
				{
					change_made |= true;
					event.m_Command = 0x80;
				}
			}

			if (!inValueOnlyAtCursor || IsCursorAtSequenceNote())
			{
				if (event.m_Note != 0x00)
				{
					change_made |= true;
					event.m_Note = 0x00;
				}
			}

			if (change_made)
				OnSequenceChanged(order_list_entry.m_SequenceIndex);

			if (!inValueOnlyAtCursor || IsCursorAtSequenceNote())
				return DoKeyDown();
		}

		return m_EventPos;
	}


	int ComponentTrack::DeleteSequenceLine(bool inChangeSequenceSize)
	{
		DataSourceOrderList::Entry order_list_entry = (*m_DataSourceOrderList)[m_EventPosDetails.OrderListIndex()];

		if (order_list_entry.m_Transposition < 0xfe)
		{
			const std::shared_ptr<DataSourceSequence>& sequence_data_source = m_DataSourceSequenceList[order_list_entry.m_SequenceIndex];

			if (!sequence_data_source->IsInErrorState())
				AddUndoStep();

			const unsigned int sequence_data_source_length = sequence_data_source->GetLength();

			if (sequence_data_source_length > 0)
			{
				for (unsigned int i = m_EventPosDetails.SequenceIndex(); i < sequence_data_source_length - 1; ++i)
				{
					const DataSourceSequence::Event& event = (*sequence_data_source)[i + 1];
					(*sequence_data_source)[i] = event;
				}

				DataSourceSequence::Event& event = (*sequence_data_source)[sequence_data_source_length - 1];

				event.m_Command = 0x80;
				event.m_Instrument = 0x80;
				event.m_Note = 0x00;

				if (inChangeSequenceSize && sequence_data_source_length > 1)
				{
					sequence_data_source->SetLength(sequence_data_source_length - 1);
					UpdateMaxEventPos();
				}

				OnSequenceChanged(order_list_entry.m_SequenceIndex);

				if (inChangeSequenceSize && sequence_data_source_length > 1)
				{
					OnOrderListChanged();
					return ComponentTrackUtils::GetEventPosOf(m_EventPosDetails.OrderListIndex(), m_EventPosDetails.SequenceIndex(), m_DataSourceOrderList, m_DataSourceSequenceList);
				}
			}
		}

		return m_EventPos;
	}

	int ComponentTrack::InsertSequenceLine(bool inChangeSequenceSize)
	{
		DataSourceOrderList::Entry order_list_entry = (*m_DataSourceOrderList)[m_EventPosDetails.OrderListIndex()];

		if (order_list_entry.m_Transposition < 0xfe)
		{
			const std::shared_ptr<DataSourceSequence>& sequence_data_source = m_DataSourceSequenceList[order_list_entry.m_SequenceIndex];
			const unsigned int sequence_data_source_length = sequence_data_source->GetLength();

			if (!sequence_data_source->IsInErrorState())
				AddUndoStep();

			if (sequence_data_source_length > 1 || inChangeSequenceSize)
			{
				bool changed_size = false;
				if (inChangeSequenceSize && sequence_data_source_length < DataSourceSequence::MaxEventCount)
				{
					sequence_data_source->SetLength(sequence_data_source_length + 1);
					changed_size = true;
				}

				for (unsigned int i = sequence_data_source_length; i > static_cast<unsigned int>(m_EventPosDetails.SequenceIndex()); --i)
				{
					const DataSourceSequence::Event& event = (*sequence_data_source)[i - 1];
					(*sequence_data_source)[i] = event;
				}

				DataSourceSequence::Event& event = (*sequence_data_source)[m_EventPosDetails.SequenceIndex()];

				event.m_Command = 0x80;
				event.m_Instrument = 0x80;
				event.m_Note = 0x00;

				if (changed_size)
					UpdateMaxEventPos();

				OnSequenceChanged(order_list_entry.m_SequenceIndex);

				if (changed_size)
				{
					OnOrderListChanged();
					return ComponentTrackUtils::GetEventPosOf(m_EventPosDetails.OrderListIndex(), m_EventPosDetails.SequenceIndex(), m_DataSourceOrderList, m_DataSourceSequenceList);
				}
			}
		}

		return m_EventPos;
	}


	int ComponentTrack::ResizeSequence(int inLength)
	{
		DataSourceOrderList::Entry order_list_entry = (*m_DataSourceOrderList)[m_EventPosDetails.OrderListIndex()];

		if (order_list_entry.m_Transposition < 0xfe && inLength > 0)
		{
			const std::shared_ptr<DataSourceSequence>& sequence_data_source = m_DataSourceSequenceList[order_list_entry.m_SequenceIndex];

			if (!sequence_data_source->IsInErrorState())
				AddUndoStep();

			const unsigned int sequence_data_source_length = sequence_data_source->GetLength();

			sequence_data_source->SetLength(inLength);

			for (int i = sequence_data_source_length; i < inLength; ++i)
			{
				DataSourceSequence::Event& event = (*sequence_data_source)[i];

				event.m_Command = 0x80;
				event.m_Instrument = 0x80;
				event.m_Note = 0x00;
			}

			UpdateMaxEventPos();

			OnSequenceChanged(order_list_entry.m_SequenceIndex);

			return ComponentTrackUtils::GetEventPosOf(m_EventPosDetails.OrderListIndex(), m_EventPosDetails.SequenceIndex(), m_DataSourceOrderList, m_DataSourceSequenceList);
		}

		return m_EventPos;
	}


	int ComponentTrack::ResizeAndReplaceData(const DataCopySequence* inSequenceData)
	{
		DataSourceOrderList::Entry order_list_entry = (*m_DataSourceOrderList)[m_EventPosDetails.OrderListIndex()];
		const int source_length = inSequenceData->GetEventCount();

		if (order_list_entry.m_Transposition < 0xfe && source_length > 0)
		{
			const std::shared_ptr<DataSourceSequence>& sequence_data_source = m_DataSourceSequenceList[order_list_entry.m_SequenceIndex];

			sequence_data_source->SetLength(source_length);

			for (int i = 0; i < source_length; ++i)
				(*sequence_data_source)[i] = (*inSequenceData)[i];

			UpdateMaxEventPos();

			OnSequenceChanged(order_list_entry.m_SequenceIndex);
			return ComponentTrackUtils::GetEventPosOf(m_EventPosDetails.OrderListIndex(), m_EventPosDetails.SequenceIndex(), m_DataSourceOrderList, m_DataSourceSequenceList);
		}

		return m_EventPos;
	}


	int ComponentTrack::ResizeAndInsertData(const DataCopySequenceEvents* inSequenceEventData)
	{
		DataSourceOrderList::Entry order_list_entry = (*m_DataSourceOrderList)[m_EventPosDetails.OrderListIndex()];
		if (order_list_entry.m_Transposition < 0xfe)
		{
			InsertSequenceLines(inSequenceEventData->GetEventCount());

			const std::shared_ptr<DataSourceSequence>& sequence_data_source = m_DataSourceSequenceList[order_list_entry.m_SequenceIndex];

			const int sequence_size = sequence_data_source->GetLength();
			int destination_index = m_EventPosDetails.SequenceIndex();

			for (int i = 0; i < static_cast<int>(inSequenceEventData->GetEventCount()); ++i)
			{
				if (i + destination_index >= sequence_size)
					break;

				(*sequence_data_source)[i + destination_index] = (*inSequenceEventData)[i];
			}

			UpdateMaxEventPos();

			OnSequenceChanged(order_list_entry.m_SequenceIndex);
			return ComponentTrackUtils::GetEventPosOf(m_EventPosDetails.OrderListIndex(), m_EventPosDetails.SequenceIndex(), m_DataSourceOrderList, m_DataSourceSequenceList);
		}

		return m_EventPos;
	}

	int ComponentTrack::InsertAndOverwriteData(const DataCopySequenceEvents* inSequenceEventData)
	{
		return m_EventPos;
	}


	int ComponentTrack::InsertSequenceLines(int inLineCount)
	{
		DataSourceOrderList::Entry order_list_entry = (*m_DataSourceOrderList)[m_EventPosDetails.OrderListIndex()];

		if (order_list_entry.m_Transposition < 0xfe && inLineCount > 0)
		{
			const std::shared_ptr<DataSourceSequence>& sequence_data_source = m_DataSourceSequenceList[order_list_entry.m_SequenceIndex];

			const unsigned int sequence_data_source_length = sequence_data_source->GetLength();

			if (sequence_data_source_length + inLineCount <= DataSourceSequence::MaxEventCount)
			{
				sequence_data_source->SetLength(sequence_data_source_length + inLineCount);

				int dest_index = sequence_data_source_length + inLineCount - 1;

				FOUNDATION_ASSERT(sequence_data_source_length > 0);
				for (int i = sequence_data_source_length - 1; i >= static_cast<int>(m_EventPosDetails.SequenceIndex()); --i, --dest_index)
				{
					DataSourceSequence::Event& event = (*sequence_data_source)[i];
					(*sequence_data_source)[dest_index] = event;
				}

				for (unsigned int i = m_EventPosDetails.SequenceIndex(); i < m_EventPosDetails.SequenceIndex() + inLineCount; ++i)
				{
					DataSourceSequence::Event& event = (*sequence_data_source)[i];

					event.m_Command = 0x80;
					event.m_Instrument = 0x80;
					event.m_Note = 0x00;
				}

				UpdateMaxEventPos();

				OnSequenceChanged(order_list_entry.m_SequenceIndex);

				return ComponentTrackUtils::GetEventPosOf(m_EventPosDetails.OrderListIndex(), m_EventPosDetails.SequenceIndex(), m_DataSourceOrderList, m_DataSourceSequenceList);
			}
		}

		return m_EventPos;
	}

	//--------------------------------------------------------------------------------------------------

	void ComponentTrack::UpdateMaxEventPos()
	{
		m_MaxEventPos = ComponentTrackUtils::GetMaxEventPosition(m_DataSourceOrderList, m_DataSourceSequenceList);
	}


	void ComponentTrack::SetEventPosDetails(unsigned int inOrderListIndex, unsigned int inSequenceIndex)
	{
		unsigned int previous_order_list_index = m_EventPosDetails.OrderListIndex();

		m_EventPosDetails.Set(inOrderListIndex, inSequenceIndex);

		if (inOrderListIndex != previous_order_list_index)
		{
			const auto& order_list_entry = (*m_DataSourceOrderList)[inOrderListIndex];
			if (order_list_entry.m_Transposition >= 0xfe)
				m_OrderListIndexChangedEvent.Execute(m_HasControl, inOrderListIndex, 0xff);
			else
				m_OrderListIndexChangedEvent.Execute(m_HasControl, inOrderListIndex, order_list_entry.m_SequenceIndex);
		}
	}

	//--------------------------------------------------------------------------------------------------
	// Information report (function callback)
	//--------------------------------------------------------------------------------------------------

	void ComponentTrack::UpdateSequenceStatusReport()
	{
		if (m_HasControl && m_StatusReportFunction != nullptr && !m_FocusModeOrderList)
		{
			DataSourceOrderList::Entry order_list_entry = (*m_DataSourceOrderList)[m_EventPosDetails.OrderListIndex()];

			if (order_list_entry.m_Transposition < 0xfe)
			{
				const std::shared_ptr<DataSourceSequence>& sequence_data_source = m_DataSourceSequenceList[order_list_entry.m_SequenceIndex];
				m_StatusReportFunction(true, order_list_entry.m_SequenceIndex, sequence_data_source->GetPackedSize());
			}
			else
				m_StatusReportFunction(true, -1, -1);
		}
	}

	void ComponentTrack::UpdateOrderListStatusReport()
	{
		if (m_HasControl && m_StatusReportFunction != nullptr && m_FocusModeOrderList)
			m_StatusReportFunction(false, 0, (*m_DataSourceOrderList).GetPackedSize());
	}

	//--------------------------------------------------------------------------------------------------
	// Cursor
	//--------------------------------------------------------------------------------------------------

	void ComponentTrack::ApplyCursorPosition(CursorControl& inCursorControl)
	{
		int actual_cursor_y = m_Position.m_Y + m_FocusRow.m_Row;
		int actual_cursor_x = m_Position.m_X + [&]()
		{
			if (m_FocusModeOrderList)
				return m_CursorPos;
			else
			{
				const int sequence_offset = 5;
				if (m_CursorPos < 2)
					return sequence_offset + m_CursorPos;
				if (m_CursorPos < 4)
					return sequence_offset + m_CursorPos + 1;

				return sequence_offset + m_CursorPos + 2;
			}
		}();

		inCursorControl.SetPosition(CursorControl::Position({ actual_cursor_x, actual_cursor_y }));
	}


	int ComponentTrack::GetCursorPositionXFromSequenceCellX(int inCellX) const
	{
		int local_sequence_cell_left_x = inCellX - (m_Position.m_X + 5);
		
		if (local_sequence_cell_left_x < 0)
			return -1;

		if (local_sequence_cell_left_x < 3)
			return std::max(0, std::min(1, local_sequence_cell_left_x));

		if (local_sequence_cell_left_x < 6)
			return std::max(2, std::min(3, local_sequence_cell_left_x - 1));

		return 4;
	}


	int ComponentTrack::GetCursorPositionXFromOrderListCellX(int inCellX) const
	{
		int local_order_list_cell_left_x = inCellX - m_Position.m_X;

		if (local_order_list_cell_left_x < 0)
			return -1;

		if (local_order_list_cell_left_x < 3)
			return local_order_list_cell_left_x;

		return 3;
	}


	//--------------------------------------------------------------------------------------------------
	// Key input logic
	//--------------------------------------------------------------------------------------------------

	void ComponentTrack::DoKeyLeft(bool inControlIsDown)
	{
		if (m_FocusModeOrderList)
		{
			if (m_CursorPos > 0)
				--m_CursorPos;
		}
		else
		{
			if (inControlIsDown)
			{
				m_CursorPos &= ~1;

				if (m_CursorPos > 0)
					m_CursorPos -= 2;
				else
					m_CursorPos = 4;
			}
			else if (m_CursorPos > 0)
				--m_CursorPos;
		}
	}


	void ComponentTrack::DoKeyRight(bool inControlIsDown)
	{
		if (m_FocusModeOrderList)
		{
			if (m_CursorPos < 3)
				m_CursorPos++;
		}
		else
		{
			if (inControlIsDown)
			{
				m_CursorPos &= ~1;

				if (m_CursorPos < 4)
					m_CursorPos += 2;
				else
					m_CursorPos = 0;
			}
			else if (m_CursorPos < 4)
				m_CursorPos++;
		}
	}


	int ComponentTrack::DoKeyDown()
	{
		if (!m_FocusModeOrderList)
		{
			if (m_EventPos < m_MaxEventPos)
				return m_EventPos + 1;
		}
		else
		{
			const unsigned char transposition = (*m_DataSourceOrderList)[m_EventPosDetails.OrderListIndex()].m_Transposition;

			if (transposition < 0xfe)
			{
				const unsigned char sequence_index = (*m_DataSourceOrderList)[m_EventPosDetails.OrderListIndex()].m_SequenceIndex;
				const int sequence_length = m_DataSourceSequenceList[sequence_index]->GetLength();

				return m_EventPos + sequence_length;
			}
		}

		return m_EventPos;
	}


	int ComponentTrack::DoKeyUp()
	{
		if (!m_FocusModeOrderList)
		{
			if (m_EventPos > 0)
				return m_EventPos - 1;
		}
		else
		{
			if (m_EventPosDetails.OrderListIndex() > 0)
			{
				const unsigned char sequence_index = (*m_DataSourceOrderList)[m_EventPosDetails.OrderListIndex() - 1].m_SequenceIndex;
				const int sequence_length = m_DataSourceSequenceList[sequence_index]->GetLength();

				return m_EventPos - sequence_length;
			}
		}

		return m_EventPos;

	}


	int ComponentTrack::DoKeyPageDown()
	{
		if (!m_FocusModeOrderList)
		{
			if (m_EventPos < m_MaxEventPos)
			{
				int event_pos = m_EventPos + page_up_down_step;
				return (event_pos > m_MaxEventPos) ? m_MaxEventPos : event_pos;
			}
		}
		else
		{
			int order_list_index = m_EventPosDetails.OrderListIndex();
			int event_pos = m_EventPos;

			for (int i = 0; i < 4; ++i)
			{
				const unsigned char transposition = (*m_DataSourceOrderList)[order_list_index].m_Transposition;

				if (transposition >= 0xfe)
					break;

				const unsigned char sequence_index = (*m_DataSourceOrderList)[order_list_index].m_SequenceIndex;
				const int sequence_length = m_DataSourceSequenceList[sequence_index]->GetLength();

				event_pos += sequence_length;
				++order_list_index;
			}

			return event_pos;
		}

		return m_EventPos;
	}


	int ComponentTrack::DoKeyPageUp()
	{
		if (!m_FocusModeOrderList)
		{
			if (m_EventPos > 0)
			{
				int event_pos = m_EventPos - page_up_down_step;
				return (event_pos < 0) ? 0 : event_pos;
			}
		}
		else
		{
			if (m_EventPosDetails.OrderListIndex() > 0)
			{
				int order_list_index = m_EventPosDetails.OrderListIndex();
				int event_pos = m_EventPos;

				for (int i = 0; i < 4; ++i)
				{
					if (order_list_index > 0)
					{
						--order_list_index;

						const unsigned char sequence_index = (*m_DataSourceOrderList)[order_list_index].m_SequenceIndex;
						const int sequence_length = m_DataSourceSequenceList[sequence_index]->GetLength();

						event_pos -= sequence_length;
					}
				}

				return event_pos;
			}
		}

		return m_EventPos;
	}



	int ComponentTrack::DoInsert(bool inIsControlDown)
	{
		if (m_FocusModeOrderList)
		{
			AddUndoStep();
			DataSourceOrderList::Entry orderlist_entry = (*m_DataSourceOrderList)[m_EventPosDetails.OrderListIndex()];

			if (orderlist_entry.m_Transposition >= 0xfe)
			{
				if (m_EventPosDetails.OrderListIndex() == 0)
					return m_EventPos;

				orderlist_entry = (*m_DataSourceOrderList)[m_EventPosDetails.OrderListIndex() - 1];
			}

			if (OrderListInsert(m_DataSourceOrderList, m_EventPosDetails.OrderListIndex(), orderlist_entry))
			{
				OnOrderListChanged();
			}

			return m_EventPos;
		}
		else
		{
			if (m_IsMarkingArea)
				DoCancelMarking();

			return InsertSequenceLine(inIsControlDown);
		}
	}


	int ComponentTrack::DoDelete(bool inIsControlDown)
	{
		if (m_FocusModeOrderList)
		{
			if (m_DataSourceOrderList->GetLength() > 2 && (*m_DataSourceOrderList)[m_EventPosDetails.OrderListIndex()].m_Transposition <= 0xfe)
			{
				AddUndoStep();

				for (int i = m_EventPosDetails.OrderListIndex(); i < static_cast<int>(m_DataSourceOrderList->GetLength()) - 1; ++i)
					(*m_DataSourceOrderList)[i] = (*m_DataSourceOrderList)[i + 1];

				m_DataSourceOrderList->DecreaseSize();

				OnOrderListChanged();
			}

			return m_EventPos;
		}
		else
		{
			if (m_IsMarkingArea)
				DoCancelMarking();

			return DeleteSequenceLine(inIsControlDown);
		}
	}


	int ComponentTrack::DoBackspace(bool inIsControlDown)
	{
		if (m_FocusModeOrderList)
		{
			if (m_EventPosDetails.OrderListIndex() > 0)
			{
				int event_pos = DoKeyUp();
				SetEventPosDetails(m_EventPosDetails.OrderListIndex() - 1, m_EventPosDetails.SequenceIndex());
				DoDelete(inIsControlDown);

				return event_pos;
			}
		}
		else
		{
			if (m_EventPosDetails.SequenceIndex() > 0)
			{
				if (m_IsMarkingArea)
					DoCancelMarking();

				int event_pos = DoKeyUp();
				SetEventPosDetails(m_EventPosDetails.OrderListIndex(), m_EventPosDetails.SequenceIndex() - 1);
				DoDelete(inIsControlDown);

				return event_pos;
			}
		}

		return m_EventPos;
	}


	int ComponentTrack::DoInsertFirstFreeSequence(const std::function<unsigned char()> inFindFreeSequence)
	{
		if (m_FocusModeOrderList)
		{
			AddUndoStep();

			DataSourceOrderList::Entry orderlist_entry = (*m_DataSourceOrderList)[m_EventPosDetails.OrderListIndex()];

			if (orderlist_entry.m_Transposition >= 0xfe)
			{
				if (m_EventPosDetails.OrderListIndex() == 0)
					return m_EventPos;

				orderlist_entry = (*m_DataSourceOrderList)[m_EventPosDetails.OrderListIndex() - 1];
			}

			unsigned char first_free_sequence_index = inFindFreeSequence();

			if (first_free_sequence_index < 0x80)
				orderlist_entry.m_SequenceIndex = first_free_sequence_index;

			if (OrderListInsert(m_DataSourceOrderList, m_EventPosDetails.OrderListIndex(), orderlist_entry))
			{
				OnOrderListChanged();
			}
		}

		return m_EventPos;
	}

	void ComponentTrack::DoSplitSequence()
	{
		if (!m_FocusModeOrderList)
		{
			if (m_IsMarkingArea)
				DoCancelMarking();

			DataSourceOrderList::Entry orderlist_entry = (*m_DataSourceOrderList)[m_EventPosDetails.OrderListIndex()];

			if (orderlist_entry.m_Transposition >= 0xfe)
				return;

			const int index_in_current_sequence = m_EventPosDetails.SequenceIndex();
			if (index_in_current_sequence == 0)
				return;

			unsigned char first_free_sequence_index = m_GetFirstEmptySequenceIndexFunction();
			if (first_free_sequence_index >= 0x80)
				return;

			AddUndoStep();

			unsigned char current_sequence_index = orderlist_entry.m_SequenceIndex;
			auto& source = m_DataSourceSequenceList[current_sequence_index];
			auto& destination = m_DataSourceSequenceList[first_free_sequence_index];

			int new_sequence_length = source->GetLength() - index_in_current_sequence;
			destination->SetLength(new_sequence_length);

			DataSourceUtils::CopySequence(source, index_in_current_sequence, new_sequence_length, destination);
			source->SetLength(index_in_current_sequence);

			OnSequenceChanged(current_sequence_index);
			OnSequenceChanged(first_free_sequence_index);

			m_SequenceSplitEvent.Execute(current_sequence_index, first_free_sequence_index);
		}
	}

	void ComponentTrack::DoDuplicateSequence(const bool inReplaceOriginal)
	{
		if (m_FocusModeOrderList)
		{
			const unsigned int order_index = m_EventPosDetails.OrderListIndex();
			DataSourceOrderList::Entry orderlist_entry = (*m_DataSourceOrderList)[order_index];

			if (orderlist_entry.m_Transposition >= 0xfe)
				return;

			// determine the empty, unused sequence to use for the duplicate
			const unsigned char duplicate_sequence_index = m_GetFirstEmptySequenceIndexFunction();
			if (duplicate_sequence_index >= 0x80)
				return;

			AddUndoStep();

			// copy current sequence to the new sequence
			const unsigned char current_sequence_index = orderlist_entry.m_SequenceIndex;
			const auto& source = m_DataSourceSequenceList[current_sequence_index];
			const auto& destination = m_DataSourceSequenceList[duplicate_sequence_index];
			const int new_sequence_length = source->GetLength();

			destination->SetLength(new_sequence_length);
			DataSourceUtils::CopySequence(source, 0, new_sequence_length, destination);
			OnSequenceChanged(duplicate_sequence_index);

			if (inReplaceOriginal)
			{
				// replace the original with the duplicate
				orderlist_entry.m_SequenceIndex = duplicate_sequence_index;
				(*m_DataSourceOrderList)[order_index] = orderlist_entry;
				OnOrderListChanged();
			}
			else
			{
				// insert the duplicate in the order list right after the original
				auto new_entry = orderlist_entry;
				new_entry.m_SequenceIndex = duplicate_sequence_index;
				if (OrderListInsert(m_DataSourceOrderList, order_index + 1, new_entry))
				{
					OnOrderListChanged();
				};
			}
		}
	}


	void ComponentTrack::DoTestExpandSequence()
	{
		if (!m_FocusModeOrderList)
		{
			DataSourceOrderList::Entry orderlist_entry = (*m_DataSourceOrderList)[m_EventPosDetails.OrderListIndex()];

			if (orderlist_entry.m_Transposition >= 0xfe)
				return;

			AddUndoStep();

			unsigned char current_sequence_index = orderlist_entry.m_SequenceIndex;
			auto& source = m_DataSourceSequenceList[current_sequence_index];

			DataSourceUtils::ExpandSequence(source);
			OnSequenceChanged(current_sequence_index);
		}
	}


	void ComponentTrack::DoResizeSequence(ComponentsManager& inComponentsManager)
	{
		if (!m_FocusModeOrderList)
		{
			if (m_IsMarkingArea)
				DoCancelMarking();

			DataSourceOrderList::Entry order_list_entry = (*m_DataSourceOrderList)[m_EventPosDetails.OrderListIndex()];

			if (order_list_entry.m_Transposition < 0xfe)
			{
				auto dialog_ok = [this](unsigned int inValue)
				{
					int event_pos = ResizeSequence(inValue);

					// Set the new event position
					if (m_EventPos != event_pos)
						SetEventPosition(event_pos);

					m_OrderListChangedEvent.Execute();
				};

				auto dialog_cancel = [this]()
				{
				};

				const unsigned int current_size = GetSequenceSize();
				std::string message = "Enter size of sequence [" + ToHexValueString(order_list_entry.m_SequenceIndex, false) + "]";

				inComponentsManager.StartDialog(std::make_shared<DialogHexValueInput>("Resize sequence", message, 30, 3, current_size, DataSourceSequence::MaxEventCount, dialog_ok, dialog_cancel));
			}
		}
	}


	void ComponentTrack::DoInsertLinesInSequence(ComponentsManager& inComponentsManager)
	{
		if (!m_FocusModeOrderList)
		{
			if (m_IsMarkingArea)
				DoCancelMarking();

			auto dialog_ok = [this](unsigned int inValue)
			{
				AddUndoStep();

				int event_pos = InsertSequenceLines(inValue);

				// Set the new event position
				if (m_EventPos != event_pos)
					SetEventPosition(event_pos);

				m_OrderListChangedEvent.Execute();
			};

			auto dialog_cancel = [this]()
			{
			};

			unsigned int max_value = DataSourceSequence::MaxEventCount - m_DataSourceSequenceList[m_EventPosDetails.OrderListIndex()]->GetLength();

			inComponentsManager.StartDialog(std::make_shared<DialogHexValueInput>("Add lines to sequence", "Enter number of lines to add to sequence", 30, 3, 0, max_value, dialog_ok, dialog_cancel));
		}
	}


	void ComponentTrack::DoCopyFullSequenceData()
	{
		FOUNDATION_ASSERT(m_FocusModeOrderList);

		DataSourceOrderList::Entry order_list_entry = (*m_DataSourceOrderList)[m_EventPosDetails.OrderListIndex()];

		if (order_list_entry.m_Transposition < 0xfe)
		{
			const std::shared_ptr<DataSourceSequence>& sequence_data_source = m_DataSourceSequenceList[order_list_entry.m_SequenceIndex];
			CopyPaste::Instance().SetData(new DataCopySequence(*sequence_data_source));
		}
	}


	void ComponentTrack::DoCopyMarkedSequenceData()
	{
		FOUNDATION_ASSERT(!m_FocusModeOrderList);

		int top = m_IsMarkingArea ? std::min(m_MarkingFromEventPos, m_MarkingToEventPos) : m_EventPos;
		int bottom = m_IsMarkingArea ? std::max(m_MarkingFromEventPos, m_MarkingToEventPos) : m_EventPos;

		// Find sequence and index of top position
		int find_event_pos = top;
		int event_pos = 0;

		int orderlist_index = 0;
		int sequence_event_pos = 0;

		bool found = false;
		
		for (unsigned int i = 0; i < m_DataSourceOrderList->GetLength(); ++i)
		{
			unsigned char sequence_index = (*m_DataSourceOrderList)[i].m_SequenceIndex;
			const std::shared_ptr<DataSourceSequence>& sequence = m_DataSourceSequenceList[sequence_index];

			int next_event_pos = event_pos + sequence->GetLength();

			if (find_event_pos >= event_pos && find_event_pos < next_event_pos)
			{
				orderlist_index = i;
				sequence_event_pos = find_event_pos - event_pos;

				found = true;
				break;
			}

			event_pos = next_event_pos;
		}

		if(found)
		{
			std::vector<DataSourceSequence::Event> events_copy;

			for (int event_pos = top; event_pos <= bottom; ++event_pos)
			{
				const auto& orderlist_entry = (*m_DataSourceOrderList)[orderlist_index];
				if (orderlist_entry.m_Transposition >= 0xfe)
					break;

				unsigned char sequence_index = orderlist_entry.m_SequenceIndex;
				const std::shared_ptr<DataSourceSequence>& sequence = m_DataSourceSequenceList[sequence_index];

				DataSourceSequence::Event event_to_copy = (*sequence)[sequence_event_pos];
				events_copy.push_back(event_to_copy);

				// Logging
				std::string output = std::to_string(event_to_copy.m_Instrument) + ": " + std::to_string(event_to_copy.m_Command) + " - " + std::to_string(event_to_copy.m_Note) + "\n";
				Logging::instance().Info(output.c_str());

				++sequence_event_pos;
				if (sequence_event_pos >= static_cast<int>(sequence->GetLength()))
				{
					++orderlist_index;
					sequence_event_pos = 0;
				}
			}

			if (!events_copy.empty())
				CopyPaste::Instance().SetData(new DataCopySequenceEvents(events_copy));
		}

		if(m_IsMarkingArea)
			DoCancelMarking();
	}


	void ComponentTrack::DoPaste(bool inResizeSequence)
	{
		if (CopyPaste::Instance().HasData<DataCopySequence>())
		{
			const auto* data = CopyPaste::Instance().GetData<DataCopySequence>();

			if (inResizeSequence)
			{
				AddUndoStep();

				int event_pos = ResizeAndReplaceData(data);

				// Set the new event position
				if (m_EventPos != event_pos)
					SetEventPosition(event_pos);

				OnOrderListChanged();
			}
		}

		if (CopyPaste::Instance().HasData<DataCopySequenceEvents>() && !m_FocusModeOrderList)
		{
			const auto* data = CopyPaste::Instance().GetData<DataCopySequenceEvents>();
			AddUndoStep();

			if (inResizeSequence)
			{
				int event_pos = ResizeAndInsertData(data);

				// Set the new event position
				if (m_EventPos != event_pos)
					SetEventPosition(event_pos);

				OnOrderListChanged();
			}
			else
			{
				int event_pos = InsertAndOverwriteData(data);

				// Set the new event position
				if (m_EventPos != event_pos)
					SetEventPosition(event_pos);

				OnOrderListChanged();
			}
		}
	}


	void ComponentTrack::DoSetInstrumentIndexValue(unsigned char inValue)
	{
		FOUNDATION_ASSERT(inValue >= 0 && inValue < 0x20);

		if (!m_FocusModeOrderList)
		{
			DataSourceOrderList::Entry order_list_entry = (*m_DataSourceOrderList)[m_EventPosDetails.OrderListIndex()];

			if (order_list_entry.m_Transposition < 0xfe)
			{
				const std::shared_ptr<DataSourceSequence>& sequence_data_source = m_DataSourceSequenceList[order_list_entry.m_SequenceIndex];
				DataSourceSequence::Event& event = (*sequence_data_source)[m_EventPosDetails.SequenceIndex()];

				if (!sequence_data_source->IsInErrorState())
					AddUndoStep();

				if (event.m_Instrument == 0x80)
					event.m_Instrument = 0xa0 | static_cast<unsigned char>(inValue);
				else
					event.m_Instrument = 0x80;

				OnSequenceChanged(order_list_entry.m_SequenceIndex);
			}
		}
	}


	void ComponentTrack::DoSetCommandIndexValue(unsigned char inValue)
	{
		FOUNDATION_ASSERT(inValue >= 0 && inValue < 0x40);

		if (!m_FocusModeOrderList)
		{
			DataSourceOrderList::Entry order_list_entry = (*m_DataSourceOrderList)[m_EventPosDetails.OrderListIndex()];

			if (order_list_entry.m_Transposition < 0xfe)
			{
				const std::shared_ptr<DataSourceSequence>& sequence_data_source = m_DataSourceSequenceList[order_list_entry.m_SequenceIndex];
				DataSourceSequence::Event& event = (*sequence_data_source)[m_EventPosDetails.SequenceIndex()];

				if (!sequence_data_source->IsInErrorState())
					AddUndoStep();

				if (event.m_Command == 0x80)
					event.m_Command = 0xc0 | static_cast<unsigned char>(inValue);
				else
					event.m_Command = 0x80;

				OnSequenceChanged(order_list_entry.m_SequenceIndex);
			}
		}
	}


	void ComponentTrack::DoBeginMarking(int inBeginMarkingEventPos)
	{
		FOUNDATION_ASSERT(!m_IsMarkingArea);

		m_MarkingFromEventPos = inBeginMarkingEventPos;
		m_IsMarkingArea = true;
	}

	void ComponentTrack::DoCancelMarking()
	{
		FOUNDATION_ASSERT(m_IsMarkingArea);

		m_IsMarkingArea = false;
	}


	//--------------------------------------------------------------------------------------------------
	// Data change
	//--------------------------------------------------------------------------------------------------

	void ComponentTrack::OnOrderListChanged()
	{
		UpdateMaxEventPos();

		// Pack orderlist
		DataSourceOrderList::PackResult packed_result = m_DataSourceOrderList->Pack();

		if (packed_result.m_DataLength < 0x100)
			m_DataSourceOrderList->SendPackedDataToBuffer(packed_result);

		UpdateOrderListStatusReport();

		m_HasDataChange = true;
		m_HasDataChangeOrderList = true;
		m_RequireRefresh = true;
	}

	void ComponentTrack::OnSequenceChanged(unsigned char inSequenceIndex)
	{
		const std::shared_ptr<DataSourceSequence>& inSequence = m_DataSourceSequenceList[inSequenceIndex];
		DataSourceSequence::PackResult packed_result = inSequence->Pack();

		m_RequireRefresh = true;

		if (packed_result.m_DataLength < 0x100 && packed_result.m_Data != nullptr)
		{
			inSequence->SendPackedDataToBuffer(packed_result);

			m_HasDataChange = true;
			m_DataChangeSequenceIndexList.push_back(inSequenceIndex);
		}

		m_SequenceDataHasChanged = true;

		UpdateSequenceStatusReport();
	}

	//--------------------------------------------------------------------------------------------------

	ComponentTrack::EventPosDetails ComponentTrack::GetEventPosDetails(int inEventPos) const
	{
		EventPosDetails details;

		int event_pos = 0;

		if (inEventPos >= m_MaxEventPos && m_MaxEventPos > 0)
		{
			const int loop_event_pos = GetLoopEventPosition();
			const int loop_size = m_MaxEventPos - loop_event_pos;

			if (loop_size > 0)
			{
				int loop_count = (inEventPos - loop_event_pos) / loop_size;
				inEventPos -= loop_count * loop_size;
			}
		}

		for (unsigned int i = 0; i < m_DataSourceOrderList->GetLength(); ++i)
		{
			unsigned char sequence_index = (*m_DataSourceOrderList)[i].m_SequenceIndex;
			const std::shared_ptr<DataSourceSequence>& sequence = m_DataSourceSequenceList[sequence_index];

			int next_event_pos = event_pos + sequence->GetLength();

			if (inEventPos >= event_pos && inEventPos < next_event_pos)
			{
				details.Set(i, static_cast<unsigned int>(inEventPos - event_pos));

				break;
			}

			event_pos = next_event_pos;
		}

		return details;
	}


	bool ComponentTrack::IsEventPosStartOfSequence(int inEventPos) const
	{
		if (inEventPos < 0 && inEventPos >= m_MaxEventPos)
			return false;

		const auto details = GetEventPosDetails(inEventPos);

		return details.SequenceIndex() == 0;
	}



	void ComponentTrack::AddUndoStep()
	{
		std::shared_ptr<UndoComponentDataTableTracks> undo_data = std::make_shared<UndoComponentDataTableTracks>();

		m_AddUndoStepHandler(*undo_data);

		undo_data->m_CursorPos = m_CursorPos;
		undo_data->m_MaxEventPos = m_MaxEventPos;
		undo_data->m_EventPosDetailsOrderListIndex = m_EventPosDetails.OrderListIndex();
		undo_data->m_EventPosDetailsSequenceIndex = m_EventPosDetails.SequenceIndex();
		undo_data->m_EventPosOrderListIndex = m_EventPosOrderListIndex;
		undo_data->m_EventPosSequenceIndex = m_EventPosSequenceIndex;

		m_Undo->AddUndo(undo_data, [this](const UndoComponentData& inData, CursorControl& inCursorControl) { this->OnUndo(inData, inCursorControl); });

		m_LocalDataChange = true;
	}


	void ComponentTrack::AddUndoRecentModificationStep(bool inLockCPU)
	{
		std::shared_ptr<UndoComponentDataTableTracks> undo_data = std::make_shared<UndoComponentDataTableTracks>();

		m_AddUndoStepHandler(*undo_data);

		undo_data->m_CursorPos = m_CursorPos;
		undo_data->m_MaxEventPos = m_MaxEventPos;
		undo_data->m_EventPosDetailsOrderListIndex = m_EventPosDetails.OrderListIndex();
		undo_data->m_EventPosDetailsSequenceIndex = m_EventPosDetails.SequenceIndex();
		undo_data->m_EventPosOrderListIndex = m_EventPosOrderListIndex;
		undo_data->m_EventPosSequenceIndex = m_EventPosSequenceIndex;

		m_Undo->AddMostRecentEdit(inLockCPU, undo_data, [this](const UndoComponentData& inData, CursorControl& inCursorControl) { this->OnUndo(inData, inCursorControl); });
	}


	void ComponentTrack::OnUndo(const UndoComponentData& inData, CursorControl& inCursorControl)
	{
		const UndoComponentDataTableTracks& undo_data = static_cast<const UndoComponentDataTableTracks&>(inData);

		if (m_TakingOrderListInput)
			CancelOrderListInputValue();

		m_CursorPos = undo_data.m_CursorPos;
		m_MaxEventPos = undo_data.m_MaxEventPos;
		m_EventPosOrderListIndex = undo_data.m_EventPosOrderListIndex;
		m_EventPosSequenceIndex = undo_data.m_EventPosSequenceIndex;
		SetEventPosDetails(undo_data.m_EventPosDetailsOrderListIndex, undo_data.m_EventPosDetailsSequenceIndex);

		m_OnUndoHandler(undo_data, inCursorControl);

		PullSequenceDataFromSource();
		PullDataFromSource(true);

		GetComputeMaxEventPosEvent().Execute();

		m_RequireRefresh = true;
	}


	void ComponentTrack::ConfigureKeyHooks(const Utility::KeyHookStore& inKeyHookStore)
	{
		using namespace Utility;

		m_KeyHooks.clear();

		m_KeyHooks.push_back({ "Key.Track.CursorUp", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			if (!m_TakingOrderListInput)
			{
				if (m_IsMarkingArea)
					DoCancelMarking();

				inKeyHookContext.m_NewEventPos = DoKeyUp();
				return true;
			}

			return false;
		} });
		m_KeyHooks.push_back({"Key.Track.CursorDown", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			if (!m_TakingOrderListInput)
			{
				if (m_IsMarkingArea)
					DoCancelMarking();

				inKeyHookContext.m_NewEventPos = DoKeyDown();
				return true;
			}

			return false;
		} });
	
		m_KeyHooks.push_back({ "Key.Track.MarkingCursorUp", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			if (!m_TakingOrderListInput)
			{
				if (!m_FocusModeOrderList)
				{
					if (!m_IsMarkingArea)
						DoBeginMarking(m_EventPos);

					int new_event_pos = DoKeyUp();

					m_MarkingToEventPos = new_event_pos;
					inKeyHookContext.m_NewEventPos = new_event_pos;

					return true;
				}
			}

			return false;
		} });
		m_KeyHooks.push_back({"Key.Track.MarkingCursorDown", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			if (!m_TakingOrderListInput)
			{
				if (!m_FocusModeOrderList)
				{
					if (!m_IsMarkingArea)
						DoBeginMarking(m_EventPos);

					int new_event_pos = DoKeyDown();

					m_MarkingToEventPos = new_event_pos;
					inKeyHookContext.m_NewEventPos = new_event_pos;

					return true;
				}
			}

			return false;
		} });

		m_KeyHooks.push_back({"Key.Track.ToggleGateUntilNextEvent", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			if (!m_FocusModeOrderList)
			{
				ToggleSequenceHoldNoteUntilEvent(true);
				return true;
			}

			return false;
		} });
		m_KeyHooks.push_back({ "Key.Track.ToggleGateSincePreviousEvent", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			if (!m_FocusModeOrderList)
			{
				ToggleSequenceHoldNoteUntilEvent(false);
				return true;
			}

			return false;
		} });
		m_KeyHooks.push_back({ "Key.Track.PageUp", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			if (!m_TakingOrderListInput)
			{
				inKeyHookContext.m_NewEventPos = DoKeyPageUp();
				return true;
			}

			return false;
		} });
		m_KeyHooks.push_back({ "Key.Track.PageDown", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			if (!m_TakingOrderListInput)
			{
				inKeyHookContext.m_NewEventPos = DoKeyPageDown();
				return true;
			}

			return false;
		} });
		m_KeyHooks.push_back({ "Key.Track.TransposeSemiToneDown", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			if (!m_TakingOrderListInput)
			{
				ApplyTranspose(-1);
				return true;
			}

			return false;
		} });
		m_KeyHooks.push_back({ "Key.Track.TransposeOctaveDown", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			if (!m_TakingOrderListInput)
			{
				ApplyTranspose(-12);
				return true;
			}

			return false;
		} });
		m_KeyHooks.push_back({ "Key.Track.TransposeSemiToneUp", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			if (!m_TakingOrderListInput)
			{
				ApplyTranspose(1);
				return true;
			}

			return false;
		} });
		m_KeyHooks.push_back({ "Key.Track.TransposeOctaveUp", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			if (!m_TakingOrderListInput)
			{
				ApplyTranspose(12);
				return true;
			}

			return false;
		} });
		m_KeyHooks.push_back({ "Key.Track.InsertEventsInSequence", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			if (!m_TakingOrderListInput)
			{
				DoInsertLinesInSequence(inKeyHookContext.m_ComponentsManager);
				return true;
			}

			return false;
		} });
		m_KeyHooks.push_back({ "Key.Track.ResizeSequence", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			if (!m_TakingOrderListInput)
			{
				DoResizeSequence(inKeyHookContext.m_ComponentsManager);
				return true;
			}

			return false;
		} });
		m_KeyHooks.push_back({ "Key.Track.EraseEventUnderCursor", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			if (!m_TakingOrderListInput)
			{
				inKeyHookContext.m_NewEventPos = EraseSequenceLine(true);
				return true;
			}

			return false;
		} });
		m_KeyHooks.push_back({ "Key.Track.EraseEventLine", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			if (!m_TakingOrderListInput)
			{
				inKeyHookContext.m_NewEventPos = EraseSequenceLine(false);
				return true;
			}

			return false;
		} });
		m_KeyHooks.push_back({ "Key.Track.ToggleGateUnderCursor", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			if (!m_TakingOrderListInput)
			{
				inKeyHookContext.m_NewEventPos = ApplySequenceHoldNoteValue();
				return true;
			}

			return false;
		} });
		m_KeyHooks.push_back({ "Key.Track.CursorLeft", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			DoKeyLeft(false);
			return true;
		} });
		m_KeyHooks.push_back({ "Key.Track.CursorRight", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			DoKeyRight(false);
			return true;
		} });
		m_KeyHooks.push_back({ "Key.Track.CursorLeftColumn", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			DoKeyLeft(true);
			return true;
		} });
		m_KeyHooks.push_back({ "Key.Track.CursorRightColumn", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			DoKeyRight(true);
			return true;
		} });
		m_KeyHooks.push_back({ "Key.Track.Insert", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			inKeyHookContext.m_NewEventPos = DoInsert(false);
			return true;
		} });
		m_KeyHooks.push_back({ "Key.Track.InsertAndGrow", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			inKeyHookContext.m_NewEventPos = DoInsert(true);
			return true;
		} });
		m_KeyHooks.push_back({ "Key.Track.Delete", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			inKeyHookContext.m_NewEventPos = DoDelete(false);
			return true;
		} });
		m_KeyHooks.push_back({ "Key.Track.InsertAndShrink", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			inKeyHookContext.m_NewEventPos = DoDelete(true);
			return true;
		} });
		m_KeyHooks.push_back({ "Key.Track.Backspace", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			inKeyHookContext.m_NewEventPos = DoBackspace(false);
			return true;
		} });
		m_KeyHooks.push_back({ "Key.Track.BackspaceAndShrink", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			inKeyHookContext.m_NewEventPos = DoBackspace(true);
			return true;
		} });
		m_KeyHooks.push_back({ "Key.Track.ShiftBackspace", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			inKeyHookContext.m_NewEventPos = DoInsert(false);
			return true;
		} });
		m_KeyHooks.push_back({ "Key.Track.ShiftBackspaceAndGrow", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			inKeyHookContext.m_NewEventPos = DoInsert(true);
			return true;
		} });
		m_KeyHooks.push_back({ "Key.Track.Home", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			if (!m_TakingOrderListInput)
			{
				inKeyHookContext.m_NewEventPos = 0;
				return true;
			}

			return false;
		} });
		m_KeyHooks.push_back({ "Key.Track.End", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			if (!m_TakingOrderListInput)
			{
				inKeyHookContext.m_NewEventPos = m_MaxEventPos;
				return true;
			}

			return false;
		} });
		m_KeyHooks.push_back({ "Key.Track.ApplyOrderlistInput", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			if (m_TakingOrderListInput)
			{
				ApplyOrderListInputValue();
				return true;
			}

			return false;
		} });
		m_KeyHooks.push_back({ "Key.Track.CancelOrderlistInput", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			if (m_TakingOrderListInput)
			{
				CancelOrderListInputValue();
				return true;
			}

			return false;
		} });
		m_KeyHooks.push_back({ "Key.Track.ToggleTieNote", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			if (!m_FocusModeOrderList)
			{
				ToggleSequenceTieNote();
				return true;
			}

			return false;
		} });
		m_KeyHooks.push_back({ "Key.Track.Copy", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			if (!m_TakingOrderListInput)
			{
				if (m_FocusModeOrderList)
					DoCopyFullSequenceData();
				else
					DoCopyMarkedSequenceData();

				m_RequireRefresh = true;
				
				return true;
			}

			return false;
		} });
		m_KeyHooks.push_back({ "Key.Track.Paste", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			if (!m_TakingOrderListInput)
			{
				DoPaste(true);

				return true;
			}

			return false;
		} });
		m_KeyHooks.push_back({ "Key.Track.InsertFirstFreeSequence", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			if (m_FocusModeOrderList && !m_TakingOrderListInput)
			{
				inKeyHookContext.m_NewEventPos = DoInsertFirstFreeSequence(m_GetFirstFreeSequenceIndexFunction);
				return true;
			}

			return false;
		} });
		m_KeyHooks.push_back({ "Key.Track.InsertFirstEmptySequence", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			if (m_FocusModeOrderList && !m_TakingOrderListInput)
			{
				inKeyHookContext.m_NewEventPos = DoInsertFirstFreeSequence(m_GetFirstEmptySequenceIndexFunction);
				return true;
			}

			return false;
		} });
		m_KeyHooks.push_back({ "Key.Track.SplitSequenceAtEventPosition", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			if (!m_FocusModeOrderList)
			{
				DoSplitSequence();
				UpdateMaxEventPos();
				SetEventPosition(m_EventPos);
				inKeyHookContext.m_NewEventPos = m_EventPos;
				return true;
			}

			return false;
		} });
		m_KeyHooks.push_back({ "Key.Track.DuplicateAndReplaceSequence", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			if (m_FocusModeOrderList && !m_TakingOrderListInput)
			{
				DoDuplicateSequence(true);
				SetEventPosition(m_EventPos);
				inKeyHookContext.m_NewEventPos = m_EventPos;
				return true;
			}

			return false;
		} });
		m_KeyHooks.push_back({ "Key.Track.DuplicateAndAppendSequence", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			if (m_FocusModeOrderList && !m_TakingOrderListInput)
			{
				DoDuplicateSequence(false);
				m_EventPos = DoKeyDown();
				SetEventPosition(m_EventPos);
				inKeyHookContext.m_NewEventPos = m_EventPos;
				return true;
			}

			return false;
		} });
		m_KeyHooks.push_back({ "Key.Track.SetSelectedInstrumentIndexValue", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			if (!m_FocusModeOrderList)
			{
				DoSetInstrumentIndexValue(m_EditState.GetSelectedInstrument());
				return true;
			}

			return false;
		} });
		m_KeyHooks.push_back({ "Key.Track.SetSelectedCommandIndexValue", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			if (!m_FocusModeOrderList)
			{
				DoSetCommandIndexValue(m_EditState.GetSelectedCommand());
				return true;
			}

			return false;
		} });

/*		m_KeyHooks.push_back({ "Key.Track.DoubleSequenceSize", SDLK_b, Keyboard::Control, [&](KeyHookContext& inKeyHookContext)
		{
			if (!m_FocusModeOrderList)
			{
				DoTestExpandSequence();
				UpdateMaxEventPos();
				SetEventPosition(m_EventPos);
				inKeyHookContext.m_NewEventPos = m_EventPos;
				return true;
			}

			return false;
		} });
*/
		m_KeyHooks.push_back({ "Key.Track.Debug", inKeyHookStore, [&](KeyHookContext& inKeyHookContext)
		{
			if (m_FocusModeOrderList)
				DebugSingleton::GetInstance()->SetDebugAddress(m_DataSourceOrderList->GetSourceAddress());
			else
			{
				const DataSourceOrderList::Entry orderlist_entry = (*m_DataSourceOrderList)[m_EventPosDetails.OrderListIndex()];
				DebugSingleton::GetInstance()->SetDebugAddress(m_DataSourceSequenceList[orderlist_entry.m_SequenceIndex]->GetSourceAddress());
			}

			return true;
		} });
	}


	std::string ComponentTrack::ToHexValueString(unsigned char inValue, const bool inUppercase)
	{
		auto make_character = [&inUppercase](unsigned char inValue) -> char
		{
			if (inValue > 0x0f)
				return 'x';
			if (inValue < 10)
				return '0' + inValue;

			if (inUppercase)
				return 'A' + inValue - 10;

			return 'a' + inValue - 10;
		};

		return std::string({ make_character(inValue >> 4), make_character(inValue & 0x0f) });
	}
}
