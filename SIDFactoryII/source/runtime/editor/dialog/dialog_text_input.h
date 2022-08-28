#pragma once

#include "dialog_message.h"
#include "runtime/editor/datasources/datasource_memory_buffer.h"
#include "runtime/editor/components/component_text_input.h"
#include "runtime/editor/components/component_button.h"

namespace Editor
{
	class DialogTextInput : public DialogMessage
	{
	public:
		DialogTextInput(
			const std::string& inCaption,
			const std::string& inMessage,
			const std::string& inTextOnInputLine,
			const std::string& inDefaultText,
			int inWidth,
			int inTextInputMaxLength,
			bool inOkAndCancelButtons,
			std::function<void(std::string)>&& inDone,
			std::function<void(void)>&& inCancel);

		void Cancel() override;
		bool ConsumeInput(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse) override;

	protected:
		virtual void ActivateInternal(Foundation::Viewport* inViewport) override;

		int GetDialogHeight() const override;

	private:
		void OnCancel();
		void OnDone();

		bool m_OkAndCancelButtons;
		int m_TextInputMaxLength;

		std::string m_TextOnInputLine;
		std::string m_DefaultInputText;

		std::function<void(std::string)> m_DoneFunction;
		std::function<void(void)> m_CancelFunction;

		std::shared_ptr<DataSourceMemoryBuffer> m_TextDataBuffer;
		std::shared_ptr<ComponentTextInput> m_TextInputComponent;
		std::shared_ptr<ComponentButton> m_ComponentButtonOk;
		std::shared_ptr<ComponentButton> m_ComponentButtonCancel;

	};
}