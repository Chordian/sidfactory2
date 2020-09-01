#pragma once

#include "dialog_message.h"
#include "runtime/editor/datasources/datasource_memory_buffer.h"
#include "runtime/editor/components/component_hex_value_input.h"

namespace Editor
{
	class DialogHexValueInput : public DialogMessage
	{
	public:
		DialogHexValueInput(const std::string& inCaption, const std::string& inMessage, int inWidth, int inDigitCount, unsigned int inInitialValue, unsigned int inMaxValue, std::function<void(unsigned int)>&& inDone, std::function<void(void)>&& inCancel);

		bool ConsumeInput(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse) override;

	protected:
		virtual void ActivateInternal(Foundation::Viewport* inViewport) override;
		virtual void Cancel() override;

		int GetDialogHeight() const override;

	private:
		const int m_DigitCount;
		const unsigned int m_InitialValue;
		const unsigned int m_MaxValue;

		std::function<void(unsigned int)> m_DoneFunction;
		std::function<void(void)> m_CancelFunction;

		std::shared_ptr<DataSourceMemoryBuffer> m_HexValueDataBuffer;
		std::shared_ptr<ComponentHexValueInput> m_HexValueInputComponent;
	};
}