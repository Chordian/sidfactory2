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
	struct UndoComponentData;

	class TextEditingDataSourceTableText
	{
	public:
		TextEditingDataSourceTableText(
			const std::shared_ptr<DataSourceTableText>& inDataSourceTableText,
			const unsigned int inMaxTextLength
		);

		bool IsEditing() const;
		bool RequireRefresh() const;
		bool HasDataChange() const;

		int GetTextLineIndex() const;

		void TrySetCursorPosition(int inCursorPosition);

		const std::string& GetText() const;

		void StartEditing(unsigned int inLineIndex, const Foundation::Point& inScreenPosition);
		void StopEditing(bool inCancel);
		void UpdateScreenPosition(const Foundation::Point& inScreenPosition, CursorControl& inCursorControl);

		bool ConsumeKeyboardInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl);

		void ApplyDataChange();
		void ResetRequireRefresh();

	private:
		void ApplyCharacter(char inCharacter);
		void ApplyCursorPosition(CursorControl& inCursorControl);

		void DoStopEditText(bool inCancel);

		void DoCursorForward();
		void DoCursorBackwards();
		void DoInsert();
		void DoDelete();
		void DoBackspace(bool inIsShiftDown);

		const int GetMaxPossibleCursorPosition() const;

		Foundation::Point m_ScreenPosition;
		int m_TextLineEditIndex;
		int m_TextEditCursorPos;
		int m_MaxTextLength;
		bool m_IsEditingText;
		bool m_HasDataChange;
		bool m_RequireRefresh;

		std::string m_TextSaved;
		std::string m_Text;
		std::shared_ptr<DataSourceTableText> m_DataSourceTableText;
	};
}