#pragma once

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
	struct Point;
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

		bool StartEditing(unsigned int inLineIndex);
		void StopEditing(bool inCancel);

		bool ConsumeKeyboardInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager);
		bool ConsumeMouseInput(Foundation::Point& inLocalCellPosition, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager);
		void ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse);

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

		int m_TextEditCursorPos;
		int m_MaxTextLength;
		bool m_EditingText;
		bool m_HasDataChangeText;

		std::string m_PreEditTextValue;
		std::shared_ptr<DataSourceTableText> m_DataSourceTableText;
	};
}