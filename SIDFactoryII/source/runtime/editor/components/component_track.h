#pragma once

#include "component_base.h"
#include "component_track_utils.h"

#include "runtime/editor/edit_state.h"
#include "runtime/editor/auxilarydata/auxilary_data_collection.h"
#include "runtime/editor/driver/idriver_architecture.h"
#include "runtime/editor/datasources/datasource_orderlist.h"
#include "runtime/editor/datasources/datasource_sequence.h"

#include "foundation/graphics/color.h"

#include "utils/event.h"
#include "SDL_keycode.h"

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>

namespace Foundation
{
	class TextField;

	struct Point;
	struct Extent;
}

namespace Utility
{
	template<typename CONTEXT>
	class KeyHook;
	class KeyHookStore;
}

namespace Editor
{	
	class UndoComponentData;
	class UndoComponentDataTableTracks;

	class DataCopySequence;
	class DataCopySequenceEvents;
	class ScreenBase;

	class ComponentTrack final : public ComponentBase
	{
		class EventPosDetails
		{
		public:
			EventPosDetails();

			void Set(unsigned int inOrderListIndex, unsigned int inSequenceIndex);

			unsigned int OrderListIndex() const;
			unsigned int SequenceIndex() const;

		private:
			unsigned int m_OrderListIndex;
			unsigned int m_SequenceIndex;
		};

		struct KeyHookContext
		{
			ComponentsManager& m_ComponentsManager;
			int& m_NewEventPos;
		};

		struct SequenceColors
		{
			Foundation::Color m_ErrorState;
			Foundation::Color m_InstrumentEmpty;
			Foundation::Color m_InstrumentEmptyFocusLine;
			Foundation::Color m_InstrumentTieNote;
			Foundation::Color m_InstrumentTieNoteFocusLine;
			Foundation::Color m_InstrumentValue;
			Foundation::Color m_InstrumentValueFocusLine;
			Foundation::Color m_InstrumentValueSelectedHighlight;
			Foundation::Color m_InstrumentValueSelectedHighlightFocusLine;
			Foundation::Color m_CommandEmpty;
			Foundation::Color m_CommandEmptyFocusLine;
			Foundation::Color m_CommandValue;
			Foundation::Color m_CommandValueFocusLine;
			Foundation::Color m_CommandValueSelectedHighlight;
			Foundation::Color m_CommandValueSelectedHighlightFocusLine;
			Foundation::Color m_NoteEmpty;
			Foundation::Color m_NoteEmptyFocusLine;
			Foundation::Color m_NoteGateOn;
			Foundation::Color m_NoteGateOnFocusLine;
			Foundation::Color m_NoteValue;
			Foundation::Color m_NoteValueFocusLine;
			Foundation::Color m_NoteValueTied;
			Foundation::Color m_NoteValueTiedFocusLine;
			Foundation::Color m_NoteValueSelectedHighlight;
			Foundation::Color m_NoteValueSelectedHighlightFocusLine;
		};

	public:
		using OrderListIndexChangedEvent = Utility::TEvent<void(bool, unsigned int, unsigned char)>;
		using SequenceSplitEvent = Utility::TEvent<void(unsigned char, unsigned char)>;
		using SequenceChangedEvent = Utility::TEvent<void(void)>;
		using OrderListChangedEvent = Utility::TEvent<void(void)>;
		using ComputeMaxEventPosEvent = Utility::TEvent<void(void)>;

		ComponentTrack(
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
		);
		~ComponentTrack();

		void SetPosition(const Foundation::Point& inPosition);
		void SetHeight(int inHeight);

		void SetHasControl(GetControlType inGetControlType, CursorControl& inCursorControl) override;
		void ClearHasControl(CursorControl& inCursorControl) override;

		bool ConsumeInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager) override;
		bool ConsumeInput(const Foundation::Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager) override;
		bool ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse) override;

		void Refresh(const DisplayState& inDisplayState) override;
		void HandleDataChange() override;
		void PullDataFromSource(const bool inFromUndo) override;
		void PullSequenceDataFromSource();

		void ExecuteInsertDeleteRule(const DriverInfo::TableInsertDeleteRule& inRule, int inSourceTableID, int inIndexPre, int inIndexPost) override;
		void ExecuteAction(int inActionInput) override;

		void ForceRefresh() override;

		int GetCursorPosition() const;
		void SetCursorPosition(int inCursorPosition);

		int GetMaxEventPosition() const;
		void SetEventPosition(int inEventPos);
		int GetEventPosition() const;
		int GetEventPositionAtTopOfCurrentSequence() const;

		void SetFocusModeOrderList(bool inFocusModeOrderList);
		bool GetFocusModeOrderList() const;
		bool IsTakingOrderListInput() const;

		bool IsMuted() const;
		void SetMuted(bool inMuted);

		void SetOrderlistLoopPoint();
		int GetLoopEventPosition() const;

		bool ConsumeHasInputCausedSequenceDataChange();
		bool ComputePlaybackStateFromEventPosition(int inEventPos, std::vector<IDriverArchitecture::PlayMarkerInfo>& outOrderListIndices) const;

		OrderListIndexChangedEvent& GetOrderListIndexChangedEvent();
		SequenceSplitEvent& GetSequenceSplitEvent();
		SequenceChangedEvent& GetSequenceChangedEvent();
		OrderListChangedEvent& GetOrderListChangedEvent();
		ComputeMaxEventPosEvent& GetComputeMaxEventPosEvent();

		void CancelOrderListInputValue();

		void SetUndoHandlers(std::function<void(UndoComponentDataTableTracks&)> inAddUndoStepHandler, std::function<void(const UndoComponentDataTableTracks&, CursorControl&)> inOnUndoHandler);
		
		// Data source
		std::shared_ptr<DataSourceOrderList> GetDataSourceOrderList();

		// Data changed
		void OnOrderListChanged();

	private:
		// Order list
		bool BeginOrderListInput();
		void ApplyOrderListValueKey(char inValueKey);
		void ApplyOrderListInputValue();

		// Sequence
		void DrawSequenceLine(int inX, int inY, const SequenceColors& inColors, const bool inIsHexUppercase, const std::shared_ptr<DataSourceSequence>& inSequence, unsigned int inSequenceIndex, int inTransposition, const bool inIsFocusLine, unsigned char& ioCurrentInstrument);

		bool IsCursorAtSequenceInstrument() const;
		bool IsCursorAtSequenceCommand() const;
		bool IsCursorAtSequenceNote() const;

		unsigned int GetSequenceSize() const;

		int ApplySequenceValueKey(char inValueKey);
		int ApplySequenceNoteValue(int inNoteValue);
		int ApplySequenceHoldNoteValue();
		void ToggleSequenceHoldNoteValueInMarkedArea();
		bool ApplyTranspose(char inDelta);
		bool ApplyTransposeInMarkedArea(char inDelta);
		void ToggleSequenceHoldNoteUntilEvent(bool inDown);
		void ToggleSequenceTieNote();
		void ToggleSequenceTieNoteInMarkedArea();
		int EraseSequenceLine(bool inValueOnlyAtCursor);
		void EraseSequenceLinesInMarkedArea(bool inValueOnlyAtCursor);

		int DeleteSequenceLine(bool inChangeSequenceSize);
		int InsertSequenceLine(bool inChangeSequenceSize);
		int ResizeSequence(int inLength);
		int ResizeAndReplaceData(const DataCopySequence* inSequenceData);
		int PasteSequenceEventData(const DataCopySequenceEvents* inSequenceEventData, bool inInsert);
		int InsertSequenceLines(int inLineCount);

		// Event position
	public:
		void UpdateMaxEventPos();

	private:
		void SetEventPosDetails(unsigned int inOrderListIndex, unsigned int inSequenceIndex);

		// Status report
		void UpdateSequenceStatusReport();
		void UpdateOrderListStatusReport();

		// Cursor
		void ApplyCursorPosition(CursorControl& inCursorControl);
		int GetCursorPositionXFromSequenceCellX(int inCellX) const;
		int GetCursorPositionXFromOrderListCellX(int inCellX) const;

		// Keys
		void DoKeyLeft(bool inControlIsDown);
		void DoKeyRight(bool inControlIsDown);
		int DoKeyDown();
		int DoKeyUp();
		int DoKeyPageDown();
		int DoKeyPageUp();
		int DoInsert(bool inIsControlDown);
		int DoDelete(bool inIsControlDown);
		int DoBackspace(bool inIsControlDown);
		int DoInsertFirstFreeSequence(const std::function<unsigned char()> inFindFreeSequence);
		void DoSplitSequence();
		void DoDuplicateSequence(const bool inReplaceOriginal);
		void DoTestExpandSequence();
		void DoResizeSequence(ComponentsManager& inComponentsManager);
		void DoInsertLinesInSequence(ComponentsManager& inComponentsManager);
		void DoCopyFullSequenceData();
		void DoCopyMarkedSequenceData();
		void DoPaste(bool inResizeSequence);
		void DoSetInstrumentIndexValue(unsigned char inValue);
		void DoSetCommandIndexValue(unsigned char inValue);

		// Marking
		void DoBeginMarking(int inBeginMarkingEventPos);
		void DoCancelMarking();

		// Data change
		void OnSequenceChanged(unsigned char inSequenceIndex);

		// Event pos details
		EventPosDetails GetEventPosDetails(int inEventPos) const;
		bool IsEventPosStartOfSequence(int inEventPos) const;

		// Undo
		void AddUndoStep();
		void AddUndoRecentModificationStep(bool inLockCPU);
		void OnUndo(const UndoComponentData& inData, CursorControl& inCursorControl);

		// Key hooks
		void ConfigureKeyHooks(const Utility::KeyHookStore& inKeyHookStore);

		static std::string ToHexValueString(unsigned char inValue, const bool inUppercase);

		template<typename PREDICATE>
		std::vector<unsigned char> ForEachEventInMarkedRange(PREDICATE&& inPredicate);

		// Edit state
		const EditState& m_EditState;

		// Auxilary song data
		const AuxilaryDataCollection& m_AuxilaryDataPlayMarkers;

		// Muted
		bool m_IsMuted;

		// Cursor and event positions
		int m_CursorPos;

		int m_EventPos;
		int m_MaxEventPos;
		EventPosDetails m_EventPosDetails;

		int m_EventPosOrderListIndex;
		int m_EventPosSequenceIndex;

		// Order list input focus
		bool m_FocusModeOrderList;
		bool m_TakingOrderListInput;
		unsigned short m_OrderListInputValue;

		// Data change
		bool m_SequenceDataHasChanged;
		bool m_HasDataChangeOrderList;
		bool m_LocalDataChange;
		std::vector<unsigned char> m_DataChangeSequenceIndexList;

		int m_TopEventPos;
		int m_HasFirstValid;
		unsigned int m_FirstValidOrderListIndex;
		unsigned int m_FirstValidSequenceIndex;

		// Marking
		bool m_IsMarkingArea;
		int m_MarkingFromEventPos;
		int m_MarkingToEventPos;

		// Undo
		std::function<void(UndoComponentDataTableTracks&)> m_AddUndoStepHandler;
		std::function<void(const UndoComponentDataTableTracks&, CursorControl&)> m_OnUndoHandler;

		ComponentTrackUtils::FocusRow m_FocusRow;

		std::shared_ptr<DataSourceOrderList> m_DataSourceOrderList;
		std::vector<std::shared_ptr<DataSourceSequence>> m_DataSourceSequenceList;

		std::function<void(bool, int, int)> m_StatusReportFunction;
		std::function<unsigned char()> m_GetFirstFreeSequenceIndexFunction;
		std::function<unsigned char()> m_GetFirstEmptySequenceIndexFunction;

		OrderListIndexChangedEvent m_OrderListIndexChangedEvent;
		SequenceSplitEvent m_SequenceSplitEvent;
		SequenceChangedEvent m_SequenceChangedEvent;
		OrderListChangedEvent m_OrderListChangedEvent;
		ComputeMaxEventPosEvent m_ComputedMaxEventPosEvent;

		// KeyHooks
		std::vector<Utility::KeyHook<bool(KeyHookContext&)>> m_KeyHooks;

		static const int page_up_down_step = 16;

		static std::string ms_NotesSharp[12];
		static std::string ms_NotesFlat[12];
	};


	template<typename PREDICATE>
	std::vector<unsigned char> ComponentTrack::ForEachEventInMarkedRange(PREDICATE&& inPredicate)
	{
		struct AlteredSequenceEvent
		{
			unsigned char m_SequenceIndex;
			int m_SequenceEventPos;
		};

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

		if (!found)
			return std::vector<unsigned char>();

		std::vector<AlteredSequenceEvent> altered_sequence_events;

		const auto is_sequence_event_already_altered = [&](unsigned char inSequenceIndex, int inSequenceEventPos)
		{
			const auto it = std::find_if(altered_sequence_events.begin(), altered_sequence_events.end(), [&](const auto& data) { return data.m_SequenceIndex == inSequenceIndex && data.m_SequenceEventPos == inSequenceEventPos; });
			return it != altered_sequence_events.end();
		};

		std::vector<unsigned char> altered_sequence_indicies;
		unsigned char last_sequence_index = 0xff;

		for (int event_pos = top, i = 0; event_pos <= bottom; ++event_pos, ++i)
		{
			const auto& orderlist_entry = (*m_DataSourceOrderList)[orderlist_index];
			if (orderlist_entry.m_Transposition >= 0xfe)
				break;

			unsigned char sequence_index = orderlist_entry.m_SequenceIndex;

			if (sequence_index != last_sequence_index)
			{
				altered_sequence_indicies.push_back(sequence_index);
				last_sequence_index = sequence_index;
			}

			const std::shared_ptr<DataSourceSequence>& sequence = m_DataSourceSequenceList[sequence_index];

			if (!is_sequence_event_already_altered(sequence_index, sequence_event_pos))
			{
				altered_sequence_events.push_back({ sequence_index, sequence_event_pos });

				DataSourceSequence::Event& event = (*sequence)[sequence_event_pos];
				inPredicate(event, i);
			}

			++sequence_event_pos;
			if (sequence_event_pos >= static_cast<int>(sequence->GetLength()))
			{
				++orderlist_index;
				sequence_event_pos = 0;
			}
		}

		return altered_sequence_indicies;
	}
}
