#pragma once

#include "foundation/base/types.h"
#include "runtime/editor/datasources/datasource_table_text.h"
#include "utils/event.h"

#include <memory>
#include <functional>

namespace Foundation
{
	class TextField;
	class TextColoring;
	class Keyboard;
	class Mouse;
}

namespace Editor
{
	class CursorControl;
	class ComponentsManager;
	class DataSourceTableText;
	class Undo;
	class UndoComponentData;

	class TextEditingDataSourceTableText
	{
	public:
		TextEditingDataSourceTableText(
			Undo* inUndo,
			int inOwningComponentID,
			int inOwningComponentGroupID,
			const std::shared_ptr<DataSourceTableText>& inDataSourceTableText,
			const unsigned int inMaxTextLength,
			const bool inDisableCursorOnStopEdit
		);

		bool IsEditing() const;
		bool RequireRefresh() const;
		bool HasDataChange() const;

		int GetTextLineIndex() const;

		void TrySetCursorPosition(int inCursorPosition);

		void StartEditing(unsigned int inLineIndex, const Foundation::Point& inScreenPosition);
		void StopEditing(bool inCancel, CursorControl& inCursorControl);
		void UpdateScreenPosition(const Foundation::Point& inScreenPosition, CursorControl& inCursorControl);

		bool ConsumeKeyboardInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl);

		void ApplyDataChange();
		void ResetRequireRefresh();

	private:
		void ApplyCharacter(char inCharacter);
		void ApplyCursorPosition(CursorControl& inCursorControl);

		void DoStopEditText(bool inCancel, CursorControl& inCursorControl);

		void DoCursorForward();
		void DoCursorBackwards();
		void DoInsert();
		void DoDelete();
		void DoBackspace(bool inIsShiftDown);

		void OnUndo(const UndoComponentData& inData, CursorControl& inCursorControl);
		void AddUndo();
		void AddMostRecentEdit();

		const int GetMaxPossibleCursorPosition() const;

		Foundation::Point m_ScreenPosition;

		int m_OwningComponentID;
		int m_OwningComponentGroupID;

		int m_TextLineEditIndex;
		int m_TextEditCursorPos;
		int m_MaxTextLength;
		bool m_IsEditingText;
		bool m_HasDataChange;
		bool m_RequireRefresh;
		bool m_DisableCursorOnStopEdit;

		Undo* m_Undo;

		std::string m_TextSaved;
		std::shared_ptr<DataSourceTableText> m_DataSourceTableText;
	};
}