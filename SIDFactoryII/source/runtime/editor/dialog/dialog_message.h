#pragma once

#include "dialog_base.h"

#include "foundation/graphics/wrapped_string.h"

#include <string>
#include <functional>

namespace Foundation
{
	class TextField;
}

namespace Editor
{
	class DialogMessage : public DialogBase
	{
	protected:
		DialogMessage(const std::string& inMessage, int inWidth);
		DialogMessage(const std::string& inCaption, const std::string& inMessage, int inWidth);
		DialogMessage(const std::string& inCaption, const std::string& inMessage, int inWidth, bool inCenterMessage);

	public:
		DialogMessage(const std::string& inMessage, int inWidth, bool inCenterMessage, std::function<void(void)>&& inContinue);
		DialogMessage(const std::string& inCaption, const std::string& inMessage, int inWidth, bool inCenterMessage, std::function<void(void)>&& inContinue);

		~DialogMessage();

		virtual bool ConsumeInput(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse) override;

	protected:
		virtual void ActivateInternal(Foundation::Viewport* inViewport) override;
		virtual void DeactivateInternal(Foundation::Viewport* inViewport) override;

		int GetMessageTopPositionY() const;
		int GetMessageBottomPositionY() const;

		virtual int GetDialogHeight() const;

		const bool m_HasCaption;
		const int m_Width;

		Foundation::TextField* m_TextField;
	
	private:
		bool m_CenterMessage;

		const std::string m_Caption;
		const Foundation::WrappedString m_Message;
		
		std::function<void(void)> m_ContinueFunction;
	};
}