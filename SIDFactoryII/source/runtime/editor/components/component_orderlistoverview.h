#pragma once

#include "component_base.h"

#include <memory>
#include <vector>
#include <functional>

namespace Foundation
{
	class TextField;
}

namespace Editor
{
	class CursorControl;
	class ScreenBase;
	class DataSourceOrderList;
	class DataSourceSequence;
	class DataSourceTableText;
	class TextEditingDataSourceTableText;

	class ComponentOrderListOverview final : public ComponentBase
	{
	public:
		ComponentOrderListOverview(
			int inID, 
			int inGroupID, 
			Undo* inUndo,
			Foundation::TextField* inTextField, 
			std::shared_ptr<DataSourceTableText> inDataSourceTableText,
			const std::vector<std::shared_ptr<DataSourceOrderList>>& inOrderLists,
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
		void ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse) override;
		
		void Refresh(const DisplayState& inDisplayState) override;
		bool HasDataChange() const override;
		void HandleDataChange() override;
		void PullDataFromSource() override;

		bool IsNoteInputSilenced() const override;

		void ExecuteInsertDeleteRule(const DriverInfo::TableInsertDeleteRule& inRule, int inSourceTableID, int inIndexPre, int inIndexPost) override;
		void ExecuteAction(int inActionInput) override;

		void TellPlaybackEventPosition(int inPlaybackEventPosition);

	private:
		void DoMouseWheel(const Foundation::Mouse& inMouse);
		bool DoCursorUp(unsigned int inSteps);
		bool DoCursorDown(unsigned int inSteps);
		bool DoHome();
		bool DoEnd();

		bool IsEditingText() const;
		void DoStartEditText(CursorControl& inCursorControl);
		void DoStopEditText(CursorControl& inCursorControl, bool inCancel);
		Foundation::Point GetEditingTextScreenPosition() const;

		void RebuildOverview();

		struct OverviewEntry
		{
			int m_EventPos;

			std::vector<int> m_SequenceIndices;
		};

		std::vector<OverviewEntry> m_Overview;

		std::unique_ptr<TextEditingDataSourceTableText> m_TextEditingDataSourceTableText;
		std::shared_ptr<DataSourceTableText> m_TableText;

		const std::vector<std::shared_ptr<DataSourceOrderList>>& m_OrderLists;
		const std::vector<std::shared_ptr<DataSourceSequence>>& m_SequenceList;

		std::function<void(int, bool)> m_SetTrackEventPosFunction;

		static int GetWidthFromChannelCount(int inChannelCount);

		int m_CursorPosition;
		int m_MaxCursorPosition;
		int m_TopPosition;
		int m_PlaybackEventPosition;

		static const int m_MarginWidth;
		static const int m_TextWidth;
	};
}
