#pragma once

#include "dialog_base.h"
#include "runtime/editor/datasources/datasource_memory_buffer.h"
#include "runtime/editor/components/component_hex_value_input.h"
#include "runtime/editor/components/component_button.h"
#include "runtime/editor/packer/packing_utils.h"

namespace Foundation
{
	class TextField;
}

namespace Editor
{
	class DialogPackingOptions : public DialogBase
	{
	public:
		DialogPackingOptions(unsigned short inInitialAddress, unsigned char inInitialZeroPage, ZeroPageRange inZeroPageRange, std::function<void(unsigned short, unsigned char)>&& inDone, std::function<void(void)>&& inCancel);

		void Cancel() override;
		bool ConsumeInput(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse) override;

	protected:
		virtual void ActivateInternal(Foundation::Viewport* inViewport) override;
		virtual void DeactivateInternal(Foundation::Viewport* inViewport) override;

		void OnDone();

		unsigned int ConvertToInt(std::shared_ptr<DataSourceMemoryBuffer>& inMemoryBuffer);

	private:
		int m_Width;
		int m_Height;

		unsigned short m_InitialAddress;
		unsigned char m_InitialZeroPage;

		Foundation::TextField* m_TextField;

		ZeroPageRange m_ZeroPageRange;

		std::function<void(unsigned short, unsigned char)> m_DoneFunction;
		std::function<void(void)> m_CancelFunction;

		std::shared_ptr<DataSourceMemoryBuffer> m_HexValueAddressDataBuffer;
		std::shared_ptr<DataSourceMemoryBuffer> m_HexValueZeroPageDataBuffer;
		std::shared_ptr<ComponentHexValueInput> m_HexValueAddressInputComponent;
		std::shared_ptr<ComponentHexValueInput> m_HexValueZeroPageInputComponent;

		std::shared_ptr<ComponentButton> m_ComponentButtonOk;
	};
}