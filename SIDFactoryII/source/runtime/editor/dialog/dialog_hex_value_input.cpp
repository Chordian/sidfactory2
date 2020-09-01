#include "dialog_hex_value_input.h"

#include "foundation/input/keyboard.h"
#include "runtime/editor/components_manager.h"
#include "utils/usercolors.h"

namespace Editor
{
	DialogHexValueInput::DialogHexValueInput(const std::string& inCaption, const std::string& inMessage, int inWidth, int inDigitCount, unsigned int inInitialValue, unsigned int inMaxValue, std::function<void(unsigned int)>&& inDone, std::function<void(void)>&& inCancel)
		: DialogMessage(inCaption, inMessage, inWidth)
		, m_DigitCount(inDigitCount)
		, m_InitialValue(inInitialValue)
		, m_MaxValue(inMaxValue)
		, m_DoneFunction(inDone)
		, m_CancelFunction(inCancel)
	{

	}


	void DialogHexValueInput::Cancel()
	{
		if (m_CancelFunction)
			m_CancelFunction();
	}



	bool DialogHexValueInput::ConsumeInput(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse)
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
					unsigned int value = 0;

					for (int i = 0; i < m_HexValueDataBuffer->GetSize(); ++i)
					{
						value <<= 8;
						value += (*m_HexValueDataBuffer)[i];
					}

					if (value > m_MaxValue)
						value = m_MaxValue;

					m_DoneFunction(value);
				}

				return true;
			}
		}

		return false;
	}


	void DialogHexValueInput::ActivateInternal(Foundation::Viewport* inViewport)
	{
		using namespace Utility;

		DialogMessage::ActivateInternal(inViewport);

		m_ComponentsManager = std::make_unique<ComponentsManager>(inViewport, m_CursorControl);
		m_ComponentsManager->SetGroupEnabledForInput(0, true);
		// m_ComponentsManager->SetGroupEnabledForTabbing(0);

		unsigned char buffer_byte_size = static_cast<unsigned char>((1 + m_DigitCount) >> 1);
		m_HexValueDataBuffer = std::make_shared<DataSourceMemoryBuffer>(buffer_byte_size);

		for (int i = 0; i < buffer_byte_size; ++i)
		{
			unsigned char value = static_cast<unsigned char>((m_InitialValue >> ((buffer_byte_size - 1 - i) * 8)) & 0xff);
			(*m_HexValueDataBuffer)[i] = value;
		}

		m_HexValueInputComponent = std::make_shared<ComponentHexValueInput>
			(
				0, 0,
				nullptr,
				m_HexValueDataBuffer,
				m_TextField,
				m_DigitCount,
				(m_Width - m_DigitCount) >> 1,
				GetMessageBottomPositionY() + 1
			);

		m_HexValueInputComponent->SetColors(ToColor(UserColor::DialogText));

		m_ComponentsManager->AddComponent(m_HexValueInputComponent);
		m_ComponentsManager->SetComponentInFocus(m_HexValueInputComponent);
	}


	int DialogHexValueInput::GetDialogHeight() const
	{
		return DialogMessage::GetDialogHeight() + 2;
	}
}
