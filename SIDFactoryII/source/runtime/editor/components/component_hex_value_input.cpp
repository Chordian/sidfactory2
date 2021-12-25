#include "component_hex_value_input.h"

#include "runtime/editor/cursor_control.h"
#include "runtime/editor/utilities/editor_utils.h"
#include "runtime/editor/display_state.h"
#include "foundation/input/keyboard.h"
#include "foundation/input/keyboard_utils.h"
#include "foundation/graphics/textfield.h"
#include "foundation/base/assert.h"

#include <algorithm>

using namespace Foundation;

namespace Editor
{
	ComponentHexValueInput::ComponentHexValueInput(int inID, int inGroupID, Undo* inUndo, std::shared_ptr<DataSourceMemoryBuffer> inDataSource, Foundation::TextField* inTextField, int inDigitCount, int inX, int inY)
		: ComponentBase(inID, inGroupID, inUndo, inTextField, inX, inY, inDigitCount, 1)
		, m_DataSource(inDataSource)
		, m_DigitCount(inDigitCount)
		, m_CursorPos(0)
		, m_MaxCursorPos(inDigitCount - 1)
		, m_TextColor(Color::White)
		, m_UseRange(false)
	{

	}

	ComponentHexValueInput::~ComponentHexValueInput()
	{

	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentHexValueInput::SetColors(const Foundation::Color& inTextColor)
	{
		m_TextColor = inTextColor;
	}


	bool ComponentHexValueInput::ConsumeInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		bool consume = false;

		if (m_HasControl)
		{
			for (auto& key_event : inKeyboard.GetKeyEventList())
			{
				const char character = Foundation::KeyboardUtils::FilterHexDigit(key_event);

				if (character != 0)
				{
					ApplyCharacter(character);
					consume = true;
				}
				else
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
					}
				}
			}

			ApplyCursorPosition(inCursorControl);
		}

		return consume;
	}

	bool ComponentHexValueInput::ConsumeInput(const Foundation::Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		return false;
	}


	bool ComponentHexValueInput::ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse)
	{
		return false;
	}


	void ComponentHexValueInput::SetAllowedRange(unsigned int inRangeLow, unsigned int inRangeHigh)
	{
		m_UseRange = true;
		m_RangeLow = inRangeLow;
		m_RangeHigh = inRangeHigh;
	}

	void ComponentHexValueInput::ClearAllowedRangeAll()
	{
		m_UseRange = false;
	}


	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentHexValueInput::Refresh(const DisplayState& inDisplayState)
	{
		if (m_RequireRefresh)
		{
			const bool is_uppercase = inDisplayState.IsHexUppercase();

			const int buffer_digit_size = m_DataSource->GetSize() << 1;
			const int first_digit = buffer_digit_size - m_DigitCount;

			std::string value_string = std::string(m_DigitCount, '0');

			for (int i = 0; i < m_DigitCount; ++i)
			{
				const int buffer_digit_index = first_digit + i;
				const int buffer_index = buffer_digit_index >> 1;
				const bool high_nibble = (buffer_digit_index & 1) == 0;

				const unsigned char buffer_value = high_nibble ?
					((*m_DataSource)[buffer_index] >> 4) : ((*m_DataSource)[buffer_index] & 0x0f);

				value_string.at(i) = EditorUtils::ConvertValueToSingleCharHexValue(buffer_value, is_uppercase);
			}

			m_TextField->Print(m_Position.m_X, m_Position.m_Y, m_TextColor, value_string);

			m_RequireRefresh = false;
		}
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentHexValueInput::HandleDataChange()
	{
		if (m_HasDataChange)
		{
			if(m_DataSource->PushDataToSource())
				m_HasDataChange = false;
		}
	}


	void ComponentHexValueInput::PullDataFromSource(const bool inFromUndo)
	{
	}


	void ComponentHexValueInput::ExecuteInsertDeleteRule(const DriverInfo::TableInsertDeleteRule& inRule, int inSourceTableID, int inIndexPre, int inIndexPost)
	{

	}


	void ComponentHexValueInput::ExecuteAction(int inActionInput)
	{

	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentHexValueInput::ApplyCharacter(char inCharacter)
	{
		const int buffer_digit_size = m_DataSource->GetSize() << 1;
		const int first_digit = buffer_digit_size - m_DigitCount;

		const int buffer_digit_index = first_digit + m_CursorPos;
		const int buffer_index = buffer_digit_index >> 1;
		const bool high_nibble = (buffer_digit_index & 1) == 0;

		const unsigned char value = EditorUtils::ConvertSingleCharHexValueToValue(inCharacter);
		const unsigned char current_value = (*m_DataSource)[buffer_index];

		if (high_nibble)
			(*m_DataSource)[buffer_index] = (current_value & 0x0f) | (value << 4);
		else
			(*m_DataSource)[buffer_index] = (current_value & 0xf0) | value;

		ClampRange();

		DoCursorForward();

		m_RequireRefresh = true;
		m_HasDataChange = true;
	}

	void ComponentHexValueInput::ApplyCursorPosition(CursorControl& inCursorControl)
	{
		int actual_cursor_x = m_Position.m_X + m_CursorPos;
		int actual_cursor_y = m_Position.m_Y;

		inCursorControl.SetPosition(CursorControl::Position({ actual_cursor_x, actual_cursor_y }));
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentHexValueInput::DoCursorForward()
	{
		if (m_CursorPos < m_MaxCursorPos)
			++m_CursorPos;
	}

	void ComponentHexValueInput::DoCursorBackwards()
	{
		if (m_CursorPos > 0)
			--m_CursorPos;
	}

	void ComponentHexValueInput::ClampRange()
	{
		if (!m_UseRange)
			return;

		// Only supported up to a value of 8 digits
		const size_t source_size = m_DataSource->GetSize();
		FOUNDATION_ASSERT(source_size <= 4);

		unsigned int value = 0;

		for (int i = 0; i < static_cast<int>(source_size); ++i)
		{
			value <<= 8;
			value += (*m_DataSource)[i];
		}

		value = std::max(m_RangeLow, std::min(m_RangeHigh, value));

		for (int i = static_cast<int>(source_size) - 1; i >= 0; --i)
		{
			(*m_DataSource)[i] = static_cast<unsigned char>(value & 0xff);
			value >>= 8;
		}
	}
}
