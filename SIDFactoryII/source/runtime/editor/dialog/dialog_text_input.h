#pragma once

#include "dialog_message.h"
#include "runtime/editor/datasources/datasource_memory_buffer.h"
#include "runtime/editor/components/component_text_input.h"

namespace Editor
{
	class DialogTextInput : public DialogMessage
	{
	public:
		DialogTextInput(const std::string& inCaption, const std::string& inMessage, int inWidth, int inMessageMaxLength, std::function<void(std::string)>&& inDone, std::function<void(void)>&& inCancel);

		void Cancel() override;
		bool ConsumeInput(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse) override;

	protected:
		virtual void ActivateInternal(Foundation::Viewport* inViewport) override;

		int GetDialogHeight() const override;

	private:
		int m_MessageMaxLength;

		std::function<void(std::string)> m_DoneFunction;
		std::function<void(void)> m_CancelFunction;

		std::shared_ptr<DataSourceMemoryBuffer> m_TextDataBuffer;
		std::shared_ptr<ComponentTextInput> m_TextInputComponent;
	};
}