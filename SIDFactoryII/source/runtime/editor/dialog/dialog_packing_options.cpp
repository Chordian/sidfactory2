#include "dialog_packing_options.h"

#include "runtime/editor/components_manager.h"
#include "foundation/graphics/viewport.h"
#include "foundation/graphics/textfield.h"
#include "foundation/base/types.h"
#include "foundation/graphics/color.h"
#include "foundation/input/keyboard.h"
#include "foundation/input/mouse.h"
#include "foundation/base/assert.h"
#include "utils/usercolors.h"

using namespace Utility;

namespace Editor
{
	using namespace Foundation;

	DialogPackingOptions::DialogPackingOptions(unsigned short inInitialAddress, unsigned char inInitialZeroPage, ZeroPageRange inZeroPageRange, std::function<void(unsigned short, unsigned char)>&& inDone, std::function<void(void)>&& inCancel)
		: m_InitialAddress(inInitialAddress)
		, m_InitialZeroPage(inInitialZeroPage)
		, m_ZeroPageRange(inZeroPageRange)
		, m_Width(0x24)
		, m_Height(5)
		, m_DoneFunction(inDone)
		, m_CancelFunction(inCancel)
	{

	}


	void DialogPackingOptions::Cancel()
	{
		if (m_CancelFunction)
			m_CancelFunction();
	}


	bool DialogPackingOptions::ConsumeInput(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse)
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
				if (!m_ComponentsManager->IsComponentInFocus(m_ComponentButtonOk->GetComponentID()))
					m_ComponentsManager->SetNextTabComponentFocus();

				return true;
			}
		}

		return false;
	}


	void DialogPackingOptions::ActivateInternal(Foundation::Viewport* inViewport)
	{
		m_TextField = inViewport->CreateTextField(2 + m_Width, 2 + m_Height, 0, 0);
		m_TextField->SetEnable(true);
		m_TextField->SetPositionToCenterOfViewport();

		m_TextField->ColorAreaBackground(ToColor(UserColor::DialogBackground));

		std::string caption = "Packing options";

		Color text_color = ToColor(UserColor::DialogText);

		const unsigned char zp_delta = m_ZeroPageRange.m_HighestZeroPage - m_ZeroPageRange.m_LowestZeroPage;
		const unsigned char zp_max = 0xff - zp_delta;

		m_TextField->ColorAreaBackground(ToColor(UserColor::DialogHeader), { {0, 0}, {2 + m_Width, 1} });
		m_TextField->Print((m_Width - static_cast<int>(caption.length())) >> 1, 0, ToColor(UserColor::DialogHeaderText), caption);
		m_TextField->Print(2, 2, text_color, "Address : $0000");
		m_TextField->Print(2, 3, text_color, "ZeroPage: $00   (Range: $02 - $fe)");
		m_TextField->PrintHexValue(33, 3, text_color, false, zp_max);

		m_ComponentsManager = std::make_unique<ComponentsManager>(inViewport, m_CursorControl);
		m_ComponentsManager->SetGroupEnabledForInput(0, true);

		unsigned char address_digit_count = 4;
		unsigned char zero_page_digit_count = 2;

		unsigned char address_buffer_byte_size = static_cast<unsigned char>((1 + address_digit_count) >> 1);
		unsigned char zero_page_buffer_byte_size = static_cast<unsigned char>((1 + zero_page_digit_count) >> 1);

		m_HexValueAddressDataBuffer = std::make_shared<DataSourceMemoryBuffer>(address_buffer_byte_size);
		m_HexValueZeroPageDataBuffer = std::make_shared<DataSourceMemoryBuffer>(zero_page_buffer_byte_size);

		// Address
		for (int i = 0; i < address_buffer_byte_size; ++i)
		{
			unsigned char value = static_cast<unsigned char>((m_InitialAddress >> ((address_buffer_byte_size - 1 - i) * 8)) & 0xff);
			(*m_HexValueAddressDataBuffer)[i] = value;
		}

		m_HexValueAddressInputComponent = std::make_shared<ComponentHexValueInput>
			(
				0, 0,
				nullptr,
				m_HexValueAddressDataBuffer,
				m_TextField,
				address_digit_count,
				13,
				2
			);

		m_HexValueAddressInputComponent->SetColors(ToColor(UserColor::DialogText));

		// ZP
		for (int i = 0; i < zero_page_buffer_byte_size; ++i)
		{
			unsigned char value = static_cast<unsigned char>((m_InitialZeroPage >> ((zero_page_buffer_byte_size - 1 - i) * 8)) & 0xff);
			(*m_HexValueZeroPageDataBuffer)[i] = value;
		}

		m_HexValueZeroPageInputComponent = std::make_shared<ComponentHexValueInput>
			(
				0, 0,
				nullptr,
				m_HexValueZeroPageDataBuffer,
				m_TextField,
				zero_page_digit_count,
				13,
				3
			);

		m_HexValueZeroPageInputComponent->SetColors(ToColor(UserColor::DialogText));

		m_ComponentsManager->AddComponent(m_HexValueAddressInputComponent);
		m_ComponentsManager->AddComponent(m_HexValueZeroPageInputComponent);

		const int button_width = 10;

		m_ComponentButtonOk = std::make_shared<ComponentButton>(3, 0, 
			nullptr,
			m_TextField, "Ok",
			(m_Width - button_width) >> 1, 
			5,
			button_width,
			[&]() { OnDone(); });
		m_ComponentsManager->AddComponent(m_ComponentButtonOk);

		m_ComponentsManager->SetComponentInFocus(m_HexValueAddressInputComponent);
	}


	void DialogPackingOptions::DeactivateInternal(Foundation::Viewport* inViewport)
	{
		inViewport->Destroy(m_TextField);
	}


	void DialogPackingOptions::OnDone()
	{
		m_Done = true;

		const unsigned int address = ConvertToInt(m_HexValueAddressDataBuffer);
		const unsigned int zero_page = ConvertToInt(m_HexValueZeroPageDataBuffer);

		const unsigned int zp_delta = static_cast<unsigned int>(m_ZeroPageRange.m_HighestZeroPage - m_ZeroPageRange.m_LowestZeroPage);
		const unsigned int zp_max = 0xff - zp_delta;

		const unsigned int range_adjusted_zero_page = zp_max < zero_page ? zp_max : zero_page;

		m_DoneFunction(static_cast<unsigned short>(address), static_cast<unsigned char>(range_adjusted_zero_page > 2 ? range_adjusted_zero_page : 2));
	}


	unsigned int DialogPackingOptions::ConvertToInt(std::shared_ptr<DataSourceMemoryBuffer>& inMemoryBuffer)
	{
		unsigned int value = 0;

		for (int i = 0; i < inMemoryBuffer->GetSize(); ++i)
		{
			value <<= 8;
			value += (*inMemoryBuffer)[i];
		}

		return value;
	}
}