#include "text_editing_data_source_table_text.h"


namespace Editor
{
	TextEditingDataSourceTableText::TextEditingDataSourceTableText(
		const std::shared_ptr<DataSourceTableText>& inDataSourceTableText,
		const unsigned int inMaxTextLength)
		: m_DataSourceTableText(inDataSourceTableText)
		, m_MaxTextLength(inMaxTextLength)
		, m_TextEditCursorPos(0)
		, m_HasDataChangeText(false)
	{

	}

	
	bool TextEditingDataSourceTableText::IsEditing() const
	{
		return false;
	}


	bool TextEditingDataSourceTableText::StartEditing(unsigned int inLineIndex)
	{
		return false;
	}


	void TextEditingDataSourceTableText::StopEditing(bool inCancel)
	{

	}


	bool TextEditingDataSourceTableText::ConsumeKeyboardInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		return false;
	}


	bool TextEditingDataSourceTableText::ConsumeMouseInput(Foundation::Point& inLocalCellPosition, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		return false;
	}


	void TextEditingDataSourceTableText::ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse)
	{

	}


	void TextEditingDataSourceTableText::ApplyCharacter(char inCharacter)
	{

	}


	void TextEditingDataSourceTableText::ApplyCursorPosition(CursorControl& inCursorControl)
	{

	}


	void TextEditingDataSourceTableText::DoStartEditText()
	{

	}


	void TextEditingDataSourceTableText::DoStopEditText(bool inCancel)
	{

	}


	void TextEditingDataSourceTableText::DoCursorForward()
	{

	}


	void TextEditingDataSourceTableText::DoCursorBackwards()
	{

	}


	void TextEditingDataSourceTableText::DoInsert()
	{

	}


	void TextEditingDataSourceTableText::DoDelete()
	{

	}


	void TextEditingDataSourceTableText::DoBackspace(bool inIsShiftDown)
	{

	}

	const int TextEditingDataSourceTableText::GetMaxPossibleCursorPosition() const
	{
		return 0;
	}
}