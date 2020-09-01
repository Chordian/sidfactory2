#include "component_text_input.h"
#include "runtime/editor/cursor_control.h"
#include "foundation/input/keyboard.h"
#include "foundation/input/keyboard_utils.h"
#include "foundation/graphics/textfield.h"

using namespace Foundation;

namespace Editor
{
	ComponentTextInput::ComponentTextInput(
		int inID, 
		int inGroupID, 
		Undo* inUndo, 
		std::shared_ptr<DataSourceMemoryBuffer> inDataSource, 
		TextField* inTextField, 
		Color inColor,
		int inX, 
		int inY, 
		int inWidth,
		bool inIsForFilename)
		: ComponentBase(inID, inGroupID, inUndo, inTextField, inX, inY, inWidth, 1)
		, m_DataSource(inDataSource)
		, m_CursorPos(0)
		, m_MaxCursorPos(inDataSource->GetSize() - 1)
		, m_IsForFilename(inIsForFilename)
		, m_TextColor(inColor)
	{
		m_TextField->ColorArea(inColor, inX, inY, inWidth, 1);
	}

	ComponentTextInput::~ComponentTextInput()
	{

	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	bool ComponentTextInput::ConsumeInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		bool consume = false;

		if (m_HasControl)
		{
			for (auto& text_event: inKeyboard.GetKeyTextList())
			{
				if ((!m_IsForFilename && KeyboardUtils::IsAcceptableInputText(text_event)) || 
					(m_IsForFilename && KeyboardUtils::IsAcceptableInputFilename(text_event)))
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
				}
			}

			ApplyCursorPosition(inCursorControl);
		}

		return consume;
	}


	bool ComponentTextInput::ConsumeInput(const Foundation::Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		return false;
	}


	void ComponentTextInput::ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse)
	{

	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentTextInput::Refresh(const DisplayState& inDisplayState)
	{
		if (m_RequireRefresh)
		{
			// Evaluate string size from buffer
			const int string_size = [this]()
			{
				for (int i = 0; i < m_DataSource->GetSize(); ++i)
				{
					if ((*m_DataSource)[i] == 0)
						return i;
				}

				return m_DataSource->GetSize();
			}();

			if (string_size > 0)
			{
				char* data_source_memory = reinterpret_cast<char*>(&(*m_DataSource)[0]);
				std::string text_string(data_source_memory, string_size);

				m_TextField->Print(m_Position.m_X, m_Position.m_Y, m_TextColor, text_string);
			}

			if (string_size < m_Dimensions.m_Width)
			{
				const int x = m_Position.m_X + string_size;
				const int w = m_Dimensions.m_Width - string_size;

				m_TextField->ClearText(x, m_Position.m_Y, w, 1);
			}

			m_RequireRefresh = false;
		}
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentTextInput::HandleDataChange()
	{
		if (m_HasDataChange)
		{
			if(m_DataSource->PushDataToSource())
				m_HasDataChange = false;
		}
	}


	void ComponentTextInput::PullDataFromSource()
	{

	}


	void ComponentTextInput::ExecuteInsertDeleteRule(const DriverInfo::TableInsertDeleteRule& inRule, int inSourceTableID, int inIndexPre, int inIndexPost)
	{

	}


	void ComponentTextInput::ExecuteAction(int inActionInput)
	{

	}



	void ComponentTextInput::SetCursorPosition(int inCursorPosition)
	{
		const int max = GetMaxPossibleCursorPosition();
		m_CursorPos = inCursorPosition < max ? inCursorPosition : max;
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentTextInput::ApplyCharacter(char inCharacter)
	{
		(*m_DataSource)[m_CursorPos] = inCharacter;

		DoCursorForward();

		m_RequireRefresh = true;
		m_HasDataChange = true;
	}

	void ComponentTextInput::ApplyCursorPosition(CursorControl& inCursorControl)
	{
		int actual_cursor_x = m_Position.m_X + m_CursorPos;
		int actual_cursor_y = m_Position.m_Y;

		inCursorControl.SetPosition(CursorControl::Position({ actual_cursor_x, actual_cursor_y }));
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentTextInput::DoCursorForward()
	{
		if (m_CursorPos < GetMaxPossibleCursorPosition())
			++m_CursorPos;
	}

	void ComponentTextInput::DoCursorBackwards()
	{
		if (m_CursorPos > 0)
			--m_CursorPos;
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentTextInput::DoInsert()
	{
		for (int i = m_DataSource->GetSize() - 2; i >= m_CursorPos; --i)
			(*m_DataSource)[i + 1] = (*m_DataSource)[i];

		(*m_DataSource)[m_CursorPos] = ' ';

		m_HasDataChange = true;
	}

	void ComponentTextInput::DoDelete()
	{
		for (int i = m_CursorPos; i < m_DataSource->GetSize() - 1; ++i)
			(*m_DataSource)[i] = (*m_DataSource)[i + 1];

		(*m_DataSource)[m_DataSource->GetSize() - 1] = 0;

		m_HasDataChange = true;
	}

	void ComponentTextInput::DoBackspace(bool inIsShiftDown)
	{
		if (inIsShiftDown)
			DoInsert();
		else if (m_CursorPos > 0)
		{
			DoCursorBackwards();
			DoDelete();
		}
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	const int ComponentTextInput::GetMaxPossibleCursorPosition() const
	{
		for (int i = 0; i < m_DataSource->GetSize(); ++i)
		{
			if ((*m_DataSource)[i] == 0)
				return i;
		}

		return m_DataSource->GetSize() - 1;
	}
}
