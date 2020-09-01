#pragma once

#include "dialog_base.h"
#include "runtime/editor/datasources/datasource_memory_buffer.h"
#include "runtime/editor/components/component_text_input.h"
#include "runtime/editor/components/component_button.h"

namespace Foundation
{
	class TextField;
}

namespace Editor
{
	class DialogSIDFileInfo : public DialogBase
	{
	public:
		DialogSIDFileInfo(std::function<void(std::string, std::string, std::string)>&& inDone, std::function<void(void)>&& inCancel);

		void Cancel() override;
		bool ConsumeInput(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse) override;

	protected:
		virtual void ActivateInternal(Foundation::Viewport* inViewport) override;
		virtual void DeactivateInternal(Foundation::Viewport* inViewport) override;

		void OnDone();

		std::string ConvertToString(std::shared_ptr<DataSourceMemoryBuffer>& inMemoryBuffer);

	private:
		int m_Width;
		int m_Height;

		int m_MessageMaxLength;

		Foundation::TextField* m_TextField;

		std::function<void(std::string, std::string, std::string)> m_DoneFunction;
		std::function<void(void)> m_CancelFunction;

		std::shared_ptr<DataSourceMemoryBuffer> m_TextDataBufferTitle;
		std::shared_ptr<DataSourceMemoryBuffer> m_TextDataBufferAuthor;
		std::shared_ptr<DataSourceMemoryBuffer> m_TextDataBufferCopyright;
		std::shared_ptr<ComponentTextInput> m_TextInputComponentTitle;
		std::shared_ptr<ComponentTextInput> m_TextInputComponentAuthor;
		std::shared_ptr<ComponentTextInput> m_TextInputComponentCopyright;
		std::shared_ptr<ComponentButton> m_ComponentButtonOk;
	};
}