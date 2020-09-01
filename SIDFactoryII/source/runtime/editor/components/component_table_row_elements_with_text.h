#pragma once

#include "component_table_row_elements.h"
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
	class DataSourceTableText;
	struct UndoComponentData;

	class ComponentTableRowElementsWithText final : public ComponentTableRowElements
	{
	public:
		ComponentTableRowElementsWithText
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
		);
		~ComponentTableRowElementsWithText();

		bool HasDataChange() const override;
		void HandleDataChange() override;

		bool IsNoteInputSilenced() const override;
		void ClearHasControl(CursorControl& inCursorControl) override;

		bool ConsumeInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager) override;
		bool ConsumeInput(const Foundation::Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager) override;
		void ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse) override;

		void Refresh(const DisplayState& inDisplayState) override;
		void PullDataFromSource() override;

		bool ContainsPosition(const Foundation::Point& inPixelPosition) const override;
	private:
		void ApplyCharacter(char inCharacter);
		void ApplyCursorPosition(CursorControl& inCursorControl);

		void DoStartEditText();
		void DoStopEditText(bool inCancel);

		void DoCursorForward();
		void DoCursorBackwards();
		void DoInsert();
		void DoDelete();
		void DoBackspace(bool inIsShiftDown);

		const int GetMaxPossibleCursorPosition() const;

		int m_CursorPos;
		int m_TextWidth;
		bool m_EditingText;
		bool m_HasDataChangeText;

		std::string m_PreEditTextValue;

		std::shared_ptr<DataSourceTableText> m_DataSourceTableText;
	};
}
