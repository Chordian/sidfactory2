#include "runtime/editor/components/utils/text_editing_data_source_table_text.h"
#include "runtime/editor/cursor_control.h"
#include "runtime/editor/undo/undo.h"
#include "runtime/editor/undo/undo_componentdata/undo_componentdata_table_text.h"
#include "foundation/input/keyboard.h"
#include "foundation/input/keyboard_utils.h"

using namespace Foundation;

namespace Editor
{
	TextEditingDataSourceTableText::TextEditingDataSourceTableText(
		Undo* inUndo,
		int inOwningComponentID,
		int inOwningComponentGroupID,
		const std::shared_ptr<DataSourceTableText>& inDataSourceTableText,
		const unsigned int inMaxTextLength,
		const bool inDisableCursorOnStopEdit)
		: m_Undo(inUndo)
		, m_OwningComponentID(inOwningComponentID)
		, m_OwningComponentGroupID(inOwningComponentGroupID)
		, m_DataSourceTableText(inDataSourceTableText)
		, m_MaxTextLength(inMaxTextLength)
		, m_TextEditCursorPos(0)
		, m_HasDataChange(false)
		, m_IsEditingText(false)
		, m_TextLineEditIndex(0)
		, m_RequireRefresh(false)
		, m_DisableCursorOnStopEdit(inDisableCursorOnStopEdit)
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


	void TextEditingDataSourceTableText::StartEditing(unsigned int inLineIndex, const Point& inScreenPosition)
	{
		FOUNDATION_ASSERT(!m_IsEditingText);

		m_ScreenPosition = inScreenPosition;
		m_TextLineEditIndex = inLineIndex;
		m_TextEditCursorPos = 0;

		m_TextSaved = (*m_DataSourceTableText)[m_TextLineEditIndex];

		m_IsEditingText = true;

		AddUndo();
	}


	void TextEditingDataSourceTableText::StopEditing(bool inCancel, CursorControl& inCursorControl)
	{
		FOUNDATION_ASSERT(m_IsEditingText);

		if (inCancel || (*m_DataSourceTableText)[m_TextLineEditIndex] == m_TextSaved)
		{
			m_Undo->DoUndo(inCursorControl);
			m_Undo->FlushForwardUndoSteps();
		}
		else
			AddMostRecentEdit();

		if (m_DisableCursorOnStopEdit)
			inCursorControl.SetEnabled(false);

		m_IsEditingText = false;
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
				DoStopEditText(false, inCursorControl);
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
				DoStopEditText(true, inCursorControl);
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


	void TextEditingDataSourceTableText::DoStopEditText(bool inCancel, CursorControl& inCursorControl)
	{
		StopEditing(inCancel, inCursorControl);
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


	void TextEditingDataSourceTableText::OnUndo(const UndoComponentData& inData, CursorControl& inCursorControl)
	{
		const UndoComponentDataTableText& undo_data = static_cast<const UndoComponentDataTableText&>(inData);

		FOUNDATION_ASSERT(undo_data.m_TextLines.size() == m_DataSourceTableText->GetSize());

		const int count = m_DataSourceTableText->GetSize();
		for (int i = 0; i < count; ++i)
		{
			auto& dest = (*m_DataSourceTableText)[i];
			dest = undo_data.m_TextLines[i];
		}

		m_IsEditingText = false;
		m_HasDataChange = true;
		m_RequireRefresh = true;
	}


	void TextEditingDataSourceTableText::AddUndo()
	{
		const int count = m_DataSourceTableText->GetSize();

		std::vector<std::string> text_lines;
		for (int i = 0; i < count; ++i)
			text_lines.push_back((*m_DataSourceTableText)[i]);

		auto undo_data = std::make_shared<UndoComponentDataTableText>();

		undo_data->m_ComponentID = m_OwningComponentID;
		undo_data->m_ComponentGroupID = m_OwningComponentGroupID;
		undo_data->m_TextLines = text_lines;

		m_Undo->AddUndo(undo_data, [this](const UndoComponentData& inData, CursorControl& inCursorControl) { this->OnUndo(inData, inCursorControl); });
	}


	void TextEditingDataSourceTableText::AddMostRecentEdit()
	{
		const int count = m_DataSourceTableText->GetSize();

		std::vector<std::string> text_lines;
		for (int i = 0; i < count; ++i)
			text_lines.push_back((*m_DataSourceTableText)[i]);

		auto undo_data = std::make_shared<UndoComponentDataTableText>();

		undo_data->m_ComponentID = m_OwningComponentID;
		undo_data->m_ComponentGroupID = m_OwningComponentGroupID;
		undo_data->m_TextLines = text_lines;

		m_Undo->AddMostRecentEdit(true, undo_data, [this](const UndoComponentData& inData, CursorControl& inCursorControl) { this->OnUndo(inData, inCursorControl); });
	}


	const int TextEditingDataSourceTableText::GetMaxPossibleCursorPosition() const
	{
		const int buffer_length = static_cast<int>((*m_DataSourceTableText)[m_TextLineEditIndex].length());
		return buffer_length >= m_MaxTextLength ? m_MaxTextLength - 1 : buffer_length;
	}
}