#pragma once

#include "component_base.h"
#include "runtime/editor/edit_state.h"
#include "utils/event.h"

#include <memory>
#include <vector>
#include <functional>

namespace Foundation
{
	class TextField;
}

namespace Utility
{
	template<typename CONTEXT>
	class KeyHook;
	class KeyHookStore;
}

namespace Editor
{
	class CursorControl;
	class ScreenBase;
	class DataSourceOrderList;
	class DataSourceSequence;
	class DataSourceTableText;
	class UndoComponentData;
	class TextEditingDataSourceTableText;

	class ComponentOrderListOverview final : public ComponentBase
	{
		struct KeyHookContext
		{
			ComponentsManager& m_ComponentsManager;
		};

		struct HoveredSequenceValue
		{
			bool operator == (const HoveredSequenceValue& InRhs) const
			{
				return m_Value == InRhs.m_Value;
			}

			bool operator != (const HoveredSequenceValue& InRhs) const
			{
				return m_Value != InRhs.m_Value;
			}
			
			void Reset()
			{
				m_Value = 0x80;
			}
			
			bool HasValue() const
			{
				return (m_Value & 0x80) == 0;
			}
			
			void SetValue(int inValue)
			{
				if(inValue >= 0 && inValue < 0x7f)
					m_Value = inValue;
				else
					m_Value = 0x80;
			}
			
			unsigned char GetValue() const
			{
				FOUNDATION_ASSERT(HasValue());
				return m_Value;
			}
		
		private:
			unsigned char m_Value = 0x80;
		};

	public:
		using OrderListChangedEvent = Utility::TEvent<void(int)>;

		ComponentOrderListOverview(
			int inID, 
			int inGroupID, 
			Undo* inUndo,
			Foundation::TextField* inTextField,
			const EditState& inEditState,
			const Utility::KeyHookStore& inKeyHookStore,
			std::shared_ptr<DataSourceTableText> inDataSourceTableText,
			std::vector<std::shared_ptr<DataSourceOrderList>>& inOrderLists,
			const std::vector<std::shared_ptr<DataSourceSequence>>& inSequenceList,
			int inX,
			int inY, 
			int inHeight,
			std::function<void(int, bool)> inSetTrackEventPosFunction
		);
		~ComponentOrderListOverview();

		void SetHasControl(GetControlType inGetControlType, CursorControl& inCursorControl) override;

		bool ConsumeInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager) override;
		bool ConsumeInput(const Foundation::Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager) override;
		bool ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse) override;
		
		void Refresh(const DisplayState& inDisplayState) override;
		bool HasDataChange() const override;
		void HandleDataChange() override;
		void PullDataFromSource(const bool inFromUndo) override;

		bool IsNoteInputSilenced() const override;
		bool IsFastForwardAllowed() const override;
		void ClearHasControl(CursorControl& inCursorControl) override;

		void ExecuteInsertDeleteRule(const DriverInfo::TableInsertDeleteRule& inRule, int inSourceTableID, int inIndexPre, int inIndexPost) override;
		void ExecuteAction(int inActionInput) override;

		void TellPlaybackEventPosition(int inPlaybackEventPosition);

		OrderListChangedEvent& GetOrderListChangedEvent();

	private:
		void DoMouseWheel(const Foundation::Mouse& inMouse);
		bool DoCursorUp(unsigned int inSteps);
		bool DoCursorDown(unsigned int inSteps);
		bool DoCursorLeft();
		bool DoCursorRight();
		bool DoHome();
		bool DoEnd();
		bool DoInsertTextRow(unsigned int inRow);
		bool DoDeleteTextRow(unsigned int inRow);
		bool DoCopy();
		bool DoPaste();

		void DoBeginMarking();
		void DoCancelMarking();
		int GetMarkingTopY() const;
		int GetMarkingBottomY() const;

		void AddUndoSequenceStep(unsigned int inChannel);
		void AddMostRecentSequenceEdit();
		void AddUndoTextStep();
		void AddMostRecentTextEdit();

		void OnUndoTextEdit(const UndoComponentData& inData, CursorControl& inCursorControl);
		void OnUndoSequenceEdit(const UndoComponentData& inData, CursorControl& inCursorControl);

		bool IsEditingText() const;
		void DoStartEditText(CursorControl& inCursorControl);
		void DoStopEditText(CursorControl& inCursorControl, bool inCancel);
		Foundation::Point GetEditingTextScreenPosition() const;

		void RebuildOverview();

		// Update hovered sequence
		bool UpdateHoveredSequence();
		
		// Key hooks
		void ConfigureKeyHooks(const Utility::KeyHookStore& inKeyHookStore);

		static int GetWidthFromChannelCount(int inChannelCount);
		static int GetOutputPositionFromCursorX(int inCursorX);

		struct OverviewEntry
		{
			struct SequenceEntry
			{
				int m_Transpose;
				int m_Index;
			};

			int m_EventPos;

			std::vector<SequenceEntry> m_SequenceEntries;
		};

		std::vector<OverviewEntry> m_Overview;

		std::unique_ptr<TextEditingDataSourceTableText> m_TextEditingDataSourceTableText;
		std::shared_ptr<DataSourceTableText> m_TableText;

		std::vector<std::shared_ptr<DataSourceOrderList>>& m_OrderLists;
		
		const std::vector<std::shared_ptr<DataSourceSequence>>& m_SequenceList;

		std::function<void(int, bool)> m_SetTrackEventPosFunction;

		// KeyHooks
		std::vector<Utility::KeyHook<bool(KeyHookContext&)>> m_KeyHooks;

		OrderListChangedEvent m_OrderListChangedEvent;

		const EditState& m_EditState;
		HoveredSequenceValue m_HoveredSequenceValue;

		int m_CursorY;
		int m_CursorX;
		int m_MaxCursorY;
		int m_MaxCursorX;
		int m_TopPosition;
		int m_PlaybackEventPosition;

		bool m_IsMarkingArea;
		int m_MarkingX;
		int m_MarkingFromY;
		int m_MarkingToY;

		int m_InvokeOrderListChangedEventChannel;

		static const int ms_MarginWidth;
		static const int ms_TextWidth;
	};
}
