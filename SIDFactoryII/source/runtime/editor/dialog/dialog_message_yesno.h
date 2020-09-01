#pragma once

#include "dialog_message.h"
#include "runtime/editor/datasources/datasource_memory_buffer.h"
#include "runtime/editor/components/component_button.h"

namespace Editor
{
	class DialogMessageYesNo : public DialogMessage
	{
	public:
		DialogMessageYesNo(const std::string& inCaption, const std::string& inMessage, int inWidth, std::function<void(void)>&& inYes, std::function<void(void)>&& inNo);
		
		void Cancel() override;
		bool ConsumeInput(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse) override;

	protected:
		virtual void ActivateInternal(Foundation::Viewport* inViewport) override;
		int GetDialogHeight() const override;

	private:
		std::function<void(void)> m_YesFunction;
		std::function<void(void)> m_NoFunction;

		std::shared_ptr<ComponentButton> m_ComponentYesButton;
		std::shared_ptr<ComponentButton> m_ComponentNoButton;
	};
}