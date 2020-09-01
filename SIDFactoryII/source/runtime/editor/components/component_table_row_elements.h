#pragma once

#include "component_base.h"
#include "runtime/editor/datasources/datasource_table.h"
#include "utils/event.h"

#include <memory>
#include <functional>

namespace Foundation
{
	class TextField;
	class TextColoring;
}

namespace Editor
{
	class CursorControl;
	struct UndoComponentData;

	class ComponentTableRowElements : public ComponentBase
	{
	public:
		using SelectedRowChangedEvent = Utility::TEvent<void(int)>;

		static int GetWidthFromColumns(int inColumnCount);

		ComponentTableRowElements
		(
			int inID, 
			int inGroupID, 
			Undo* inUndo, 
			std::shared_ptr<DataSourceTable> inDataSource, 
			Foundation::TextField* inTextField, 
			int inX, 
			int inY, 
			int inHeight,
			bool inIndexAsContinuousMemory
		);
		~ComponentTableRowElements();

		void ClearHasControl(CursorControl& inCursorControl) override;

		bool ConsumeInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager) override;
		bool ConsumeInput(const Foundation::Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager) override;
		void ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse) override;
		
		void Refresh(const DisplayState& inDisplayState) override;
		void HandleDataChange() override;
		void PullDataFromSource() override;

		void ExecuteInsertDeleteRule(const DriverInfo::TableInsertDeleteRule& inRule, int inSourceTableID, int inIndexPre, int inIndexPost) override;
		void ExecuteAction(int inActionInput) override;

		void EnableInsertDelete(bool inEnabled);
		void SetInsertDeleteCallback(std::function<void(int, int, int)> inInsertDeleteCallback);
		
		void SetTableActionRules(const DriverInfo::TableActionRules& inActionRules);
		void SetActionRuleExecutionCallback(std::function<void(int, int)> inActionRuleCallback);
		void SetSpaceBarPressedCallback(std::function<void(bool, bool)> inSpaceBarPressedCallback);

		const int GetSelectedRow() const;
		SelectedRowChangedEvent& GetSelectedRowChangedEvent();

		const DataSourceTable* GetDataSource() const;

		void DoCursorUp();
		void DoCursorDown();

	private:
		void DoScrollWheel(const Foundation::Mouse& inMouse, CursorControl* inCursorControl);
		void AdjustVisibleArea();

		void ApplyValueKey(char inValueKey);
		void ApplyCursorPosition(CursorControl& inCursorControl);

		void DoInsert();
		void DoDelete();
		void DoBackspace(bool inIsShiftDown);
		void DoAction();
		void DoEditBits(ComponentsManager& inComponentsManager);

		void AddUndoStep();
		void AddUndoRecentModificationStep(bool inLockCPU);
		void OnUndo(const UndoComponentData& inData, CursorControl& inCursorControl);

		void OnSpaceBarPressed(bool inPressed, bool inIsControlPressed);

		int GetLastUnusedRow();

		void SetRow(int inRow);

	protected:
		Foundation::TextColoring GetTextColoring(int inDataOffset, const Foundation::TextColoring& inBaseTextColoring) const;

		bool m_InsertDeleteEnabled;
		unsigned int m_TopRow;

		int m_CursorX;
		int m_CursorY;
		int m_MaxCursorX;
		int m_MaxCursorY;

		bool m_HasLocalDataChange;
		bool m_SpaceBarPressed;

		SelectedRowChangedEvent m_SelectedRowChangedEvent;

		DriverInfo::TableActionRules m_ActionRules;
		bool m_IndexAsContinuousMemory;

		std::function<void(int, int)> m_ActionRuleExecutionCallback;
		std::function<void(int, int, int)> m_InsertDeleteCallback;
		std::function<void(bool, bool)> m_SpaceBarCallback;

		std::shared_ptr<DataSourceTable> m_DataSource;
	};
}
