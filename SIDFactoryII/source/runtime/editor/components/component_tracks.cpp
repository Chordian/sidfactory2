#include "component_tracks.h"
#include "component_track.h"

#include "foundation/graphics/textfield.h"
#include "foundation/graphics/color.h"
#include "foundation/input/keyboard.h"
#include "foundation/input/mouse.h"

#include "runtime/editor/auxilarydata/auxilary_data_collection.h"
#include "runtime/editor/auxilarydata/auxilary_data_editing_preferences.h"
#include "runtime/editor/undo/undo_componentdata_tracks.h"
#include "runtime/editor/display_state.h"

#include "utils/usercolors.h"

#include <assert.h>

//---------------------------------------------------------------------------------------------------------

using namespace Foundation;
using namespace Editor::ComponentTrackUtils;
using namespace Utility;

namespace Editor
{
	ComponentTracks::ComponentTracks(
		int inID, 
		int inGroupID, 
		Undo* inUndo, 
		std::shared_ptr<DataSourceTrackComponents> inDataSource, 
		TextField* inTextField, 
		const AuxilaryDataCollection& inAuxilaryDataCollection, 
		const EditState& inEditState,
		int inX, 
		int inY, 
		int inHeight)
		: ComponentBase
		(
			inID,
			inGroupID,
			inUndo,
			inTextField,
			inX,
			inY,
			4 + inDataSource->GetSize() * ((*inDataSource)[0]->GetDimensions().m_Width + 1),
			inHeight
		)
		, m_DataSource(inDataSource)
		, m_AuxilaryData(inAuxilaryDataCollection)
		, m_EditState(inEditState)
		, m_EventPos(0)
		, m_PlaybackEventPosition(-1)
		, m_FocusTrackIndex(0)
		, m_FocusModeOrderList(false)
		, m_TracksPositionY(1)
		, m_TracksHeight(m_Dimensions.m_Height - m_TracksPositionY)
	{
		m_MaxEventPos = GetMaxEventPosition();
		m_FocusRow = CalculateFocusRow(m_TracksHeight >> 1, m_TracksHeight);

		// Set first track position 5 cells to the right, because the first 5 cells are used for the event number column
		Point track_position = m_Position + Point(5, m_TracksPositionY);

		// Set position and height of each track
		for (int i = 0; i < inDataSource->GetSize(); ++i)
		{
			(*inDataSource)[i]->SetPosition(track_position);
			(*inDataSource)[i]->SetHeight(m_TracksHeight);

			track_position.m_X += (*inDataSource)[i]->GetDimensions().m_Width + 1;

			// And add sequence changed event delegate
			(*inDataSource)[i]->GetSequenceChangedEvent().Add(this, Utility::TDelegate<void(void)>([&]() { AlignTracks(); }));

			// And add sequence split event delegate
			(*inDataSource)[i]->GetSequenceSplitEvent().Add(this, Utility::TDelegate<void(unsigned char, unsigned char)>([&](unsigned char inSequence, unsigned char inSequenceToAdd) { HandleSequenceSplit(inSequence, inSequenceToAdd); }));
		}

		// Set the event position on each track
		for (int i = 0; i < m_DataSource->GetSize(); ++i)
			(*m_DataSource)[i]->SetEventPosition(m_EventPos);

		// Set undo handlers
		for (int i = 0; i < m_DataSource->GetSize(); ++i)
		{
			(*m_DataSource)[i]->SetUndoHandlers(
				[this](UndoComponentDataTableTracks& ioData) { AddUndoStep(ioData); },
				[this](const UndoComponentDataTableTracks& inData, CursorControl& inCursorControl) { OnUndo(inData, inCursorControl); }
			);
		}
	}


	ComponentTracks::~ComponentTracks()
	{

	}

	//---------------------------------------------------------------------------------------------------------

	bool ComponentTracks::HasDataChange() const
	{
		for (int i = 0; i < m_DataSource->GetSize(); ++i)
		{
			if ((*m_DataSource)[i]->HasDataChange())
				return true;
		}

		return false;
	}

	//---------------------------------------------------------------------------------------------------------

	bool ComponentTracks::MayTabOutOfFocus(bool inForward) const
	{
		if (!(*m_DataSource)[m_FocusTrackIndex]->IsTakingOrderListInput())
		{
			if (!inForward && m_FocusTrackIndex == 0)
				return true;
			if (inForward && m_FocusTrackIndex == m_DataSource->GetSize() - 1)
				return true;
		}

		return false;
	}


	void ComponentTracks::SetHasControl(GetControlType inGetControlType, CursorControl& inCursorControl)
	{
		if (inGetControlType == GetControlType::Tabbed_Forward)
			m_FocusTrackIndex = 0;
		if (inGetControlType == GetControlType::Tabbed_Backward)
			m_FocusTrackIndex = m_DataSource->GetSize() - 1;

		(*m_DataSource)[m_FocusTrackIndex]->SetHasControl(inGetControlType, inCursorControl);

		m_HasControl = true;
		m_RequireRefresh = true;
	}


	void ComponentTracks::ClearHasControl(CursorControl& inCursorControl)
	{
		(*m_DataSource)[m_FocusTrackIndex]->CancelOrderListInputValue();
		(*m_DataSource)[m_FocusTrackIndex]->ClearHasControl(inCursorControl);

		ComponentBase::ClearHasControl(inCursorControl);
	}


	bool ComponentTracks::ConsumeInput(const Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		bool consume = false;

		if (m_HasControl)
		{
			for (auto& key_event : inKeyboard.GetKeyEventList())
			{
				switch (key_event)
				{
				case SDLK_TAB:
					(*m_DataSource)[m_FocusTrackIndex]->CancelOrderListInputValue();
					{
						unsigned int modifiers = inKeyboard.GetModiferMask();
						if ((modifiers & ~Keyboard::Shift) == 0)
						{
							if ((modifiers & Keyboard::Shift) == 0)
								OnTabForward(inCursorControl);
							else
								OnTabBackward(inCursorControl);
						}
					}
					break;
				case SDLK_RETURN:
					if (inKeyboard.IsModifierEmpty() && !m_EditState.IsPreventingSequenceEdit())
					{
						// Swap between orderlist and sequence input, unless the orderlist is currently being edited
						if (!(*m_DataSource)[m_FocusTrackIndex]->IsTakingOrderListInput())
						{
							m_FocusModeOrderList = !m_FocusModeOrderList;

							for (int i = 0; i < m_DataSource->GetSize(); ++i)
								(*m_DataSource)[i]->SetFocusModeOrderList(m_FocusModeOrderList);
						}
					}

					break;
				}
			}

			// Consume input on focus track
//			if (!m_EditState.IsPreventingSequenceEditing())
			{
				consume |= (*m_DataSource)[m_FocusTrackIndex]->ConsumeInput(inKeyboard, inCursorControl, inComponentsManager);
			}

			// Syncronize event track position
			int focus_track_event_pos = (*m_DataSource)[m_FocusTrackIndex]->GetEventPosition();
			if (m_EventPos != focus_track_event_pos)
			{
				for (int i = 0; i < m_DataSource->GetSize(); ++i)
				{
					if (i != m_FocusTrackIndex)
						(*m_DataSource)[i]->SetEventPosition(focus_track_event_pos);
				}

				m_EventPos = focus_track_event_pos;
				m_RequireRefresh = true;
			}
		}

		return consume;
	}


	bool ComponentTracks::ConsumeInput(const Foundation::Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		if (inMouse.IsButtonPressed(Foundation::Mouse::Button::Left))
		{
			Foundation::Point screen_position = inMouse.GetPosition();

			int index = -1;

			for (int i = 0; i < m_DataSource->GetSize(); ++i)
			{
				if ((*m_DataSource)[i]->ContainsPosition(screen_position))
				{
					index = i;
					break;
				}
			}

			if (index != -1 && m_FocusTrackIndex != index)
			{
				const int cursor_position = (*m_DataSource)[m_FocusTrackIndex]->GetCursorPosition();
				(*m_DataSource)[m_FocusTrackIndex]->ClearHasControl(inCursorControl);

				// Set next track index
				m_FocusTrackIndex = index;

				(*m_DataSource)[m_FocusTrackIndex]->SetHasControl(GetControlType::Tabbed_Forward, inCursorControl);
				(*m_DataSource)[m_FocusTrackIndex]->SetCursorPosition(cursor_position);

				if (m_FocusModeOrderList)
					(*m_DataSource)[m_FocusTrackIndex]->SetFocusModeOrderList(true);

				m_HasControl = true;
				m_RequireRefresh = true;
			}
		}

		return (*m_DataSource)[m_FocusTrackIndex]->ConsumeInput(inMouse, inModifierKeyMask, inCursorControl, inComponentsManager);
	}


	void ComponentTracks::ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse)
	{
		Point scroll_wheel = inMouse.GetWheelDelta();

		if (scroll_wheel.m_Y != 0 && (!m_HasControl || !m_FocusModeOrderList))
		{
			Foundation::Point screen_position = inMouse.GetPosition();
			if (ContainsPosition(screen_position))
			{
				int event_pos = m_EventPos;
				int change = scroll_wheel.m_Y;

				while (change > 0)
				{
					event_pos -= 2;
					--change;
				}
				while (change < 0)
				{
					event_pos += 2;
					++change;
				}

				event_pos = event_pos < 0 ? 0 : event_pos > m_MaxEventPos ? m_MaxEventPos : event_pos;

				if (event_pos != m_EventPos)
					SetEventPosition(event_pos, false);
			}
		}
	}



	void ComponentTracks::Refresh(const DisplayState& inDisplayState)
	{
		// If the max event position changed, we need to update the event track numbers!
		const int max_event_pos = GetMaxEventPosition();

		if (m_MaxEventPos != max_event_pos)
		{
			m_MaxEventPos = max_event_pos;
			m_RequireRefresh = true;
		}

		if (m_RequireRefresh)
		{
			const bool is_uppercase = inDisplayState.IsHexUppercase();

			// Refresh the component tracks
			m_RequireRefresh = false;

			// Draw event numbers
			int top_event = m_EventPos - m_FocusRow.m_RowsAbove;
			int bottom_event = top_event + m_TracksHeight;

			int current_y = m_Position.m_Y + m_TracksPositionY;
			int current_event = top_event;

			// Clear empty area if the top event is negative
			if (top_event < 0)
			{
				int blank_height = -top_event;
				m_TextField->ClearText(m_Position.m_X, current_y, m_Dimensions.m_Width, blank_height);

				current_y += blank_height;
				current_event += blank_height;
			}

			// Draw event numbers
			m_TextField->ColorAreaBackground(ToColor(UserColor::TracksEventNumbersBackground), { m_Position + Point(0, 1), { 4, m_Dimensions.m_Height - 1 } });

			const auto& editing_preferences = m_AuxilaryData.GetEditingPreferences();

			const int highlight_offset = editing_preferences.GetEventPosHighlightOffset();
			const int highlight_interval = editing_preferences.GetEventPosHighlightInterval();

			while (current_event < bottom_event)
			{
				const bool highlight = highlight_interval == 0 || ((current_event - highlight_offset) % highlight_interval) == 0;
				const bool focus_line = current_event == m_EventPos;

				TextColoring color = { (highlight ? ToColor(focus_line ? UserColor::TracksEventNumbersHighlightFocusLine : UserColor::TracksEventNumbersHighlight) : ToColor(focus_line ? UserColor::TracksEventNumbersFocusLine : UserColor::TracksEventNumbersDefault)),
					ToColor(current_event == m_PlaybackEventPosition ? UserColor::TracksEventNumbersPlaybackMarker : UserColor::TracksEventNumbersBackground) };

				m_TextField->PrintHexValue(
					m_Position.m_X,
					current_y,
					color,
					is_uppercase,
					static_cast<unsigned short>(current_event));

				++current_event;
				++current_y;

				if (current_event > m_MaxEventPos)
					break;
			}

			// Clear the underlying event number out of range
			if (current_event < bottom_event)
			{
				int blank_height = bottom_event - current_event;
				m_TextField->ClearText(m_Position.m_X, current_y, m_Dimensions.m_Width, blank_height);
			}

			const Color color_name_selected_focus = ToColor(UserColor::TrackNameSelectedFocus);
			const Color color_name_focus = ToColor(UserColor::TrackNameFocus);
			const Color color_name_selected = ToColor(UserColor::TrackNameSelected);
			const Color color_name = ToColor(UserColor::TrackName);

			for (int i = 0; i < m_DataSource->GetSize(); ++i)
			{
				const auto& point = (*m_DataSource)[i]->GetPosition() + Point(0, -1);
				std::string track_string = "Track " + std::to_string(i + 1);
				Color color = m_HasControl
					? (i == m_FocusTrackIndex ? color_name_selected_focus : color_name_focus)
					: (i == m_FocusTrackIndex ? color_name_selected : color_name);
				m_TextField->Print(point, color, track_string);
			}
		}

		// Refresh tracks
		for (int i = 0; i < m_DataSource->GetSize(); ++i)
			(*m_DataSource)[i]->Refresh(inDisplayState);
	}


	void ComponentTracks::HandleDataChange()
	{
		for (int i = 0; i < m_DataSource->GetSize(); ++i)
		{
			if ((*m_DataSource)[i]->HasDataChange())
				(*m_DataSource)[i]->HandleDataChange();
		}
	}


	void ComponentTracks::PullDataFromSource()
	{
		for (int i = 0; i < m_DataSource->GetSize(); ++i)
			(*m_DataSource)[i]->PullDataFromSource();
	}


	void ComponentTracks::ExecuteInsertDeleteRule(const DriverInfo::TableInsertDeleteRule& inRule, int inSourceTableID, int inIndexPre, int inIndexPost)
	{

	}


	void ComponentTracks::ExecuteAction(int inActionInput)
	{

	}



	void ComponentTracks::ForceRefresh()
	{
		m_RequireRefresh = true;

		for (int i = 0; i < m_DataSource->GetSize(); ++i)
			(*m_DataSource)[i]->ForceRefresh();
	}


	const bool ComponentTracks::IsMuted(int inTrack) const
	{
		assert(inTrack >= 0 && inTrack < m_DataSource->GetSize());
		return (*m_DataSource)[inTrack]->IsMuted();
	}


	void ComponentTracks::SetMuted(int inTrack, bool inMuted)
	{
		assert(inTrack >= 0 && inTrack < m_DataSource->GetSize());
		(*m_DataSource)[inTrack]->SetMuted(inMuted);
	}



	int ComponentTracks::GetEventPosition() const
	{
		return m_EventPos;
	}


	int ComponentTracks::GetEventPositionAtTopOfCurrentSequence() const
	{
		int track_in_focus = GetFocusTrackIndex();

		if (track_in_focus >= 0 && track_in_focus <= 2)
			return (*m_DataSource)[track_in_focus]->GetEventPositionAtTopOfCurrentSequence();

		return 0;
	}


	void ComponentTracks::SetEventPosition(int inEventPos, bool inCancelOrderlistFocus)
	{
		assert(inEventPos >= 0);
		m_EventPos = inEventPos < m_MaxEventPos ? inEventPos : m_MaxEventPos;

		// Set the event position on each track
		for (int i = 0; i < m_DataSource->GetSize(); ++i)
		{
			if (inCancelOrderlistFocus)
				(*m_DataSource)[i]->SetFocusModeOrderList(false);

			(*m_DataSource)[i]->SetEventPosition(m_EventPos);
		}

		if(inCancelOrderlistFocus)
			m_FocusModeOrderList = false;

		m_RequireRefresh = true;
	}


	int ComponentTracks::GetMaxEventPosition() const
	{
		int max_event_pos = 0;

		for (int i = 0; i < m_DataSource->GetSize(); ++i)
		{
			int max_event_pos_in_track = (*m_DataSource)[i]->GetMaxEventPosition();
			if (max_event_pos_in_track > max_event_pos)
				max_event_pos = max_event_pos_in_track;
		}

		return max_event_pos;
	}


	int ComponentTracks::GetLoopEventPosition() const
	{
		int min_loop_position = m_MaxEventPos;

		for (int i = 0; i < m_DataSource->GetSize(); ++i)
		{
			const int loop_position = (*m_DataSource)[i]->GetLoopEventPosition();
			if (loop_position < min_loop_position)
				min_loop_position = loop_position;
		}

		return min_loop_position;
	}


	int ComponentTracks::GetFocusTrackIndex() const
	{
		return m_FocusTrackIndex;
	}


	void ComponentTracks::SetOrderlistLoopPointAll()
	{
		for (int i = 0; i < m_DataSource->GetSize(); ++i)
			(*m_DataSource)[i]->SetOrderlistLoopPoint();
	}


	void ComponentTracks::CancelFocusModeOrderList() 
	{
		if (m_FocusModeOrderList)
		{
			m_FocusModeOrderList = false;

			for (int i = 0; i < m_DataSource->GetSize(); ++i)
				(*m_DataSource)[i]->SetFocusModeOrderList(false);
		}
	}


	void ComponentTracks::TellPlaybackEventPosition(int inPlaybackEventPosition)
	{
		if (m_PlaybackEventPosition != inPlaybackEventPosition)
		{
			m_PlaybackEventPosition = inPlaybackEventPosition;
			m_RequireRefresh = true;
		}
	}


	bool ComponentTracks::ComputePlaybackStateFromEventPosition(int inEventPos, std::vector<IDriverArchitecture::PlayMarkerInfo>& inPlayMarkerInfoList) const
	{
		for (int i = 0; i < m_DataSource->GetSize(); ++i)
		{
			if (!(*m_DataSource)[i]->ComputePlaybackStateFromEventPosition(inEventPos, inPlayMarkerInfoList))
				return false;
		}

		return true;
	}


	void ComponentTracks::AlignTracks()
	{
		m_RequireRefresh = true;

		for (int i = 0; i < m_DataSource->GetSize(); ++i)
		{
			if (i != m_FocusTrackIndex)
				(*m_DataSource)[i]->ForceRefresh();
		}
	}


	void ComponentTracks::HandleSequenceSplit(unsigned char inSequence, unsigned char inSequenceToAdd)
	{
		assert(inSequenceToAdd >= 0 && inSequenceToAdd < 0x80);

		m_RequireRefresh = true;

		for (int i = 0; i < m_DataSource->GetSize(); ++i)
			(*m_DataSource)[i]->HandleOrderListUpdateAfterSequenceSplit(inSequence, static_cast<unsigned char>(inSequenceToAdd));
	}


	void ComponentTracks::OnTabForward(CursorControl& inCursorControl)
	{
		if (m_FocusTrackIndex < m_DataSource->GetSize() - 1)
		{
			const int cursor_position = (*m_DataSource)[m_FocusTrackIndex]->GetCursorPosition();
			(*m_DataSource)[m_FocusTrackIndex]->ClearHasControl(inCursorControl);

			// Set next track index
			++m_FocusTrackIndex;

			(*m_DataSource)[m_FocusTrackIndex]->SetHasControl(GetControlType::Tabbed_Forward, inCursorControl);
			(*m_DataSource)[m_FocusTrackIndex]->SetCursorPosition(cursor_position);

			if (m_FocusModeOrderList)
				(*m_DataSource)[m_FocusTrackIndex]->SetFocusModeOrderList(true);

			m_HasControl = true;
			m_RequireRefresh = true;
		}
	}


	void ComponentTracks::OnTabBackward(CursorControl& inCursorControl)
	{
		if (m_FocusTrackIndex > 0)
		{
			const int cursor_position = (*m_DataSource)[m_FocusTrackIndex]->GetCursorPosition();
			(*m_DataSource)[m_FocusTrackIndex]->ClearHasControl(inCursorControl);

			// Set next track index
			--m_FocusTrackIndex;

			(*m_DataSource)[m_FocusTrackIndex]->SetHasControl(GetControlType::Tabbed_Backward, inCursorControl);
			(*m_DataSource)[m_FocusTrackIndex]->SetCursorPosition(cursor_position);

			if (m_FocusModeOrderList)
				(*m_DataSource)[m_FocusTrackIndex]->SetFocusModeOrderList(true);


			m_HasControl = true;
			m_RequireRefresh = true;
		}
	}

	//----------------------------------------------------------------------------------------------------
	// Undo
	//----------------------------------------------------------------------------------------------------

	void ComponentTracks::AddUndoStep(UndoComponentDataTableTracks& ioData)
	{
		ioData.m_ComponentGroupID = m_ComponentGroupID;
		ioData.m_ComponentID = m_ComponentID;

		ioData.m_TracksEventPos = m_EventPos;
		ioData.m_TracksMaxEventPos = m_MaxEventPos;
		ioData.m_FocusTrackIndex = m_FocusTrackIndex;
		ioData.m_TracksFocusModeOrderList = m_FocusModeOrderList;
	}


	void ComponentTracks::OnUndo(const UndoComponentDataTableTracks& inData, CursorControl &inCursorControl)
	{
		m_MaxEventPos = inData.m_TracksMaxEventPos;
		m_FocusModeOrderList = inData.m_TracksFocusModeOrderList;

		SetEventPosition(inData.m_TracksEventPos, false);

		if (m_FocusTrackIndex != inData.m_FocusTrackIndex)
		{
			const int cursor_position = (*m_DataSource)[m_FocusTrackIndex]->GetCursorPosition();
			(*m_DataSource)[m_FocusTrackIndex]->ClearHasControl(inCursorControl);

			m_FocusTrackIndex = inData.m_FocusTrackIndex;

			(*m_DataSource)[m_FocusTrackIndex]->SetHasControl(GetControlType::Default, inCursorControl);
			(*m_DataSource)[m_FocusTrackIndex]->SetCursorPosition(cursor_position);

			if (m_FocusModeOrderList)
				(*m_DataSource)[m_FocusTrackIndex]->SetFocusModeOrderList(true);
		}

		m_RequireRefresh = true;
	}
}
