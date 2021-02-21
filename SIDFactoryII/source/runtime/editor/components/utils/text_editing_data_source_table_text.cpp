#include "runtime/editor/components/utils/text_editing_data_source_table_text.h"
#include "runtime/editor/cursor_control.h"
#include "foundation/input/keyboard.h"
#include "foundation/input/keyboard_utils.h"

using namespace Foundation;

namespace Editor
{
	TextEditingDataSourceTableText::TextEditingDataSourceTableText(
		const std::shared_ptr<DataSourceTableText>& inDataSourceTableText,
		const unsigned int inMaxTextLength)
		: m_DataSourceTableText(inDataSourceTableText)
		, m_MaxTextLength(inMaxTextLength)
		, m_TextEditCursorPos(0)
		, m_HasDataChange(false)
		, m_IsEditingText(false)
		, m_TextLineEditIndex(0)
		, m_RequireRefresh(false)
	{

	}

	
	bool TextEditingDataSourceTableText::IsEditing() const
	{
		return m_IsEditingText;
	}


	bool TextEditingDataSourceTableText::RequireRefresh() const
	{
		return m_RequireRefresh;
	}


	bool TextEditingDataSourceTableText::HasDataChange() const
	{
		return m_HasDataChange;
	}


	int TextEditingDataSourceTableText::GetTextLineIndex() const
	{
		return m_TextLineEditIndex;
	}


	void TextEditingDataSourceTableText::TrySetCursorPosition(int inCursorPosition)
	{
		FOUNDATION_ASSERT(inCursorPosition >= 0);
		m_TextEditCursorPos = inCursorPosition > GetMaxPossibleCursorPosition() ? GetMaxPossibleCursorPosition() : inCursorPosition;
	}


	const std::string& TextEditingDataSourceTableText::GetText() const
	{
		return m_Text;
	}


	void TextEditingDataSourceTableText::StartEditing(unsigned int inLineIndex, const Point& inScreenPosition)
	{
		FOUNDATION_ASSERT(!m_IsEditingText);

		m_ScreenPosition = inScreenPosition;
		m_TextLineEditIndex = inLineIndex;
		m_TextEditCursorPos = 0;

		m_IsEditingText = true;
	}


	void TextEditingDataSourceTableText::StopEditing(bool inCancel)
	{
		FOUNDATION_ASSERT(m_IsEditingText);

		if (inCancel)
		{
			if (inCancel)
				(*m_DataSourceTableText)[m_TextLineEditIndex] = m_TextSaved;
		}

		m_IsEditingText = false;
		m_HasDataChange |= !inCancel;
	}


	void TextEditingDataSourceTableText::UpdateScreenPosition(const Foundation::Point& inScreenPosition, CursorControl& inCursorControl)
	{
		FOUNDATION_ASSERT(m_IsEditingText);

		m_ScreenPosition = inScreenPosition;
		ApplyCursorPosition(inCursorControl);
	}


	void TextEditingDataSourceTableText::ApplyDataChange()
	{
		FOUNDATION_ASSERT(m_HasDataChange);

		m_DataSourceTableText->PushDataToSource();
		m_HasDataChange = false;
	}


	void TextEditingDataSourceTableText::ResetRequireRefresh()
	{
		m_RequireRefresh = false;
	}


	bool TextEditingDataSourceTableText::ConsumeKeyboardInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl)
	{
		bool consume = false;

		for (auto& text_event : inKeyboard.GetKeyTextList())
		{
			if (KeyboardUtils::IsAcceptableInputText(text_event))
			{
				ApplyCharacter(text_event);
				consume = true;
			}
		}

		for (auto& key_event : inKeyboard.GetKeyEventList())
		{
			switch (key_event)
			{
			case SDLK_LEFT:
				DoCursorBackwards();
				consume = true;
				break;
			case SDLK_RIGHT:
				DoCursorForward();
				consume = true;
				break;
			case SDLK_INSERT:
				DoInsert();
				m_RequireRefresh = true;
				consume = true;
				break;
			case SDLK_DELETE:
				DoDelete();
				m_RequireRefresh = true;
				consume = true;
				break;
			case SDLK_BACKSPACE:
				DoBackspace(inKeyboard.IsModifierDownExclusive(Keyboard::Shift));
				m_RequireRefresh = true;
				consume = true;
				break;
			case SDLK_RETURN:
				DoStopEditText(false);
				m_RequireRefresh = true;
				consume = true;
				break;
			case SDLK_HOME:
				m_TextEditCursorPos = 0;
				m_RequireRefresh = true;
				consume = true;
				break;
			case SDLK_END:
				m_TextEditCursorPos = GetMaxPossibleCursorPosition();
				m_RequireRefresh = true;
				consume = true;
				break;
			case SDLK_ESCAPE:
				DoStopEditText(true);
				m_RequireRefresh = true;
				consume = true;
				break;
			}
		}

		ApplyCursorPosition(inCursorControl);

		return consume;
	}


	void TextEditingDataSourceTableText::ApplyCharacter(char inCharacter)
	{
		std::string& text = (*m_DataSourceTableText)[m_TextLineEditIndex];
		const int text_length = static_cast<int>(text.length());

		if (m_TextEditCursorPos < text_length)
			text[m_TextEditCursorPos] = inCharacter;
		else if (m_TextEditCursorPos == text_length && text_length < m_MaxTextLength)
			text += inCharacter;

		DoCursorForward();

		m_RequireRefresh = true;
		m_HasDataChange = true;
	}


	void TextEditingDataSourceTableText::ApplyCursorPosition(CursorControl& inCursorControl)
	{
		int actual_cursor_x = m_ScreenPosition.m_X + m_TextEditCursorPos;
		int actual_cursor_y = m_ScreenPosition.m_Y;

		inCursorControl.SetPosition(CursorControl::Position({ actual_cursor_x, actual_cursor_y }));
	}


	void TextEditingDataSourceTableText::DoStopEditText(bool inCancel)
	{
		StopEditing(inCancel);
	}


	void TextEditingDataSourceTableText::DoCursorForward()
	{
		if (m_TextEditCursorPos < GetMaxPossibleCursorPosition())
			++m_TextEditCursorPos;
	}


	void TextEditingDataSourceTableText::DoCursorBackwards()
	{
		if (m_TextEditCursorPos > 0)
			--m_TextEditCursorPos;
	}


	void TextEditingDataSourceTableText::DoInsert()
	{
		std::string& text = (*m_DataSourceTableText)[m_TextLineEditIndex];
		const int text_length = static_cast<int>(text.length());

		const auto last_character = text[text_length - 1];

		for (int i = text_length - 2; i >= m_TextEditCursorPos; --i)
			text[i + 1] = text[i];

		text[m_TextEditCursorPos] = ' ';

		if (text_length < m_MaxTextLength)
			text += last_character;

		m_HasDataChange = true;
	}


	void TextEditingDataSourceTableText::DoDelete()
	{
		std::string& text = (*m_DataSourceTableText)[m_TextLineEditIndex];
		const int text_length = static_cast<int>(text.length());

		std::string new_text = text.substr(0, m_TextEditCursorPos);
		if (m_TextEditCursorPos < text_length)
			new_text += text.substr(m_TextEditCursorPos + 1, text_length - m_TextEditCursorPos);

		text = new_text;

		m_HasDataChange = true;
	}


	void TextEditingDataSourceTableText::DoBackspace(bool inIsShiftDown)
	{
		if (inIsShiftDown)
			DoInsert();
		else if (m_TextEditCursorPos > 0)
		{
			DoCursorBackwards();
			DoDelete();
		}
	}


	const int TextEditingDataSourceTableText::GetMaxPossibleCursorPosition() const
	{
		const int buffer_length = static_cast<int>((*m_DataSourceTableText)[m_TextLineEditIndex].length());
		return buffer_length >= m_MaxTextLength ? m_MaxTextLength - 1 : buffer_length;
	}
}