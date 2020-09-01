#include "dialog_text_input.h"

#include "foundation/input/keyboard.h"
#include "runtime/editor/components_manager.h"
#include "utils/usercolors.h"

using namespace Utility;

namespace Editor
{
	DialogTextInput::DialogTextInput(const std::string& inCaption, const std::string& inMessage, int inWidth, int inMessageMaxLength, std::function<void(std::string)>&& inDone, std::function<void(void)>&& inCancel)
		: DialogMessage(inCaption, inMessage, inWidth)
		, m_MessageMaxLength(inMessageMaxLength)
		, m_DoneFunction(inDone)
		, m_CancelFunction(inCancel)
	{

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
				m_Done = true;
				m_CancelFunction();

				return true;
			case SDLK_RETURN:
				{
					m_Done = true;

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
				}
				return true;
			}
		}

		return false;
	}


	void DialogTextInput::ActivateInternal(Foundation::Viewport* inViewport)
	{
		DialogMessage::ActivateInternal(inViewport);

		m_ComponentsManager = std::make_unique<ComponentsManager>(inViewport, m_CursorControl);
		m_ComponentsManager->SetGroupEnabledForInput(0, true);
		// m_ComponentsManager->SetGroupEnabledForTabbing(0);

		m_TextDataBuffer = std::make_shared<DataSourceMemoryBuffer>(m_MessageMaxLength);
		m_TextInputComponent = std::make_shared<ComponentTextInput>
			(
				0, 0,
				nullptr,
				m_TextDataBuffer,
				m_TextField,
				ToColor(UserColor::DialogText),
				1,
				GetMessageBottomPositionY() + 1,
				m_MessageMaxLength,
				false
			);

		m_ComponentsManager->AddComponent(m_TextInputComponent);
		m_ComponentsManager->SetComponentInFocus(m_TextInputComponent);
	}


	int DialogTextInput::GetDialogHeight() const
	{
		return DialogMessage::GetDialogHeight() + 2;
	}
}