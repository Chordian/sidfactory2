#include "dialog_text_input.h"

#include "foundation/input/keyboard.h"
#include "foundation/graphics/viewport.h"
#include "foundation/graphics/textfield.h"
#include "foundation/graphics/color.h"
#include "foundation/base/assert.h"

#include "runtime/editor/components_manager.h"
#include "utils/usercolors.h"

using namespace Utility;

namespace Editor
{
	using namespace Foundation;

	DialogTextInput::DialogTextInput(
		const std::string& inCaption, 
		const std::string& inMessage,
		const std::string& inTextOnInputLine,
		const std::string& inDefaultText, 
		int inWidth, 
		int inTextInputMaxLength, 
		bool inOkAndCancelButtons, 
		std::function<void(std::string)>&& inDone, 
		std::function<void(void)>&& inCancel)
		: DialogMessage(inCaption, inMessage, inWidth)
		, m_TextOnInputLine(inTextOnInputLine)
		, m_DefaultInputText(inDefaultText)
		, m_TextInputMaxLength(inTextInputMaxLength)
		, m_OkAndCancelButtons(inOkAndCancelButtons)
		, m_DoneFunction(inDone)
		, m_CancelFunction(inCancel)
	{
		FOUNDATION_ASSERT(m_DefaultInputText.size() <= static_cast<size_t>(inTextInputMaxLength));
	}


	void DialogTextInput::Cancel()
	{
		if (m_CancelFunction)
			m_CancelFunction();
	}


	bool DialogTextInput::ConsumeInput(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse)
	{
		DialogBase::ConsumeInput(inKeyboard, inMouse);

		for (const auto& key_event : inKeyboard.GetKeyEventList())
		{
			switch (key_event)
			{
			case SDLK_ESCAPE:
				OnCancel();
				return true;
			case SDLK_RETURN:
				OnDone();
				return true;
			}
		}

		return false;
	}


	void DialogTextInput::ActivateInternal(Foundation::Viewport* inViewport)
	{
		DialogMessage::ActivateInternal(inViewport);

		Color text_color = ToColor(UserColor::DialogHeaderText);
		Color text_background_color = ToColor(UserColor::DialogHeader);

		const size_t text_on_input_line_length = m_TextOnInputLine.size();
		const size_t default_input_text_length = m_DefaultInputText.size();

		m_TextField->ColorAreaBackground(text_background_color, 1, GetMessageBottomPositionY() + 1, m_Width, 1);

		if(text_on_input_line_length > 0)
			m_TextField->Print(2, GetMessageBottomPositionY() + 1, text_color, m_TextOnInputLine);

		m_ComponentsManager = std::make_unique<ComponentsManager>(inViewport, m_CursorControl);
		m_ComponentsManager->SetGroupEnabledForInput(0, true);

		m_TextDataBuffer = std::make_shared<DataSourceMemoryBuffer>(m_TextInputMaxLength);
		FOUNDATION_ASSERT(default_input_text_length < static_cast<size_t>(m_TextInputMaxLength));
		for (size_t i = 0; i < default_input_text_length; ++i)
			(*m_TextDataBuffer)[i] = m_DefaultInputText.c_str()[i];

		m_TextInputComponent = std::make_shared<ComponentTextInput>
			(
				0, 0,
				nullptr,
				m_TextDataBuffer,
				m_TextField,
				ToColor(UserColor::DialogHeaderText),
				(text_on_input_line_length == 0) ? 1 : (text_on_input_line_length + 2),
				GetMessageBottomPositionY() + 1,
				m_TextInputMaxLength,
				false
			);

		m_TextInputComponent->SetCursorPosition(default_input_text_length);

		m_ComponentsManager->AddComponent(m_TextInputComponent);

		if (m_OkAndCancelButtons)
		{
			const int button_width = 10;

			m_ComponentButtonOk = std::make_shared<ComponentButton>(3, 0,
				nullptr,
				m_TextField, "Ok",
				1, 
				GetMessageBottomPositionY() + 3,
				button_width,
				[&]() { OnDone(); });
			m_ComponentsManager->AddComponent(m_ComponentButtonOk);

			m_ComponentButtonCancel = std::make_shared<ComponentButton>(3, 0,
				nullptr,
				m_TextField, "Cancel",
				m_Width - (button_width + 1),
				GetMessageBottomPositionY() + 3,
				button_width,
				[&]() { OnCancel(); });
			m_ComponentsManager->AddComponent(m_ComponentButtonCancel);
		}

		m_ComponentsManager->SetComponentInFocus(m_TextInputComponent);
	}


	int DialogTextInput::GetDialogHeight() const
	{
		return DialogMessage::GetDialogHeight() + (m_OkAndCancelButtons ? 4 : 2);
	}


	void DialogTextInput::OnDone()
	{
		const int text_length = [this]()
		{
			const int buffer_size = m_TextDataBuffer->GetSize();

			for (int i = 0; i < buffer_size; ++i)
			{
				if ((*m_TextDataBuffer)[i] == 0)
					return i;
			}

			return buffer_size;
		}();

		if (text_length > 0)
		{
			char* text_buffer = new char[text_length];

			for (int i = 0; i < text_length; ++i)
				text_buffer[i] = static_cast<char>((*m_TextDataBuffer)[i]);

			m_DoneFunction(std::string(text_buffer, text_length));
			delete[] text_buffer;
		}
		else
			m_CancelFunction();

		m_Done = true;
	}

	void DialogTextInput::OnCancel()
	{
		m_CancelFunction();

		m_Done = true;
	}
}