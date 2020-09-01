#pragma once

#include "dialog_base.h"
#include "runtime/editor/datasources/datasource_memory_buffer.h"
#include <functional>

namespace Foundation
{
	class TextField;
}

namespace Editor
{
	class DialogBitInput : public DialogBase
	{
	public:
		DialogBitInput(unsigned char inValue, std::function<void(unsigned char)>&& inDone, std::function<void(void)>&& inCancel);

		bool ConsumeInput(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse) override;

	protected:
		virtual void ActivateInternal(Foundation::Viewport* inViewport) override;
		virtual void DeactivateInternal(Foundation::Viewport* inViewport) override;
		virtual void Cancel() override;

	private:
		unsigned char m_Value;

		int m_Width;
		int m_Height;

		Foundation::TextField* m_TextField;

		std::function<void(unsigned char)> m_DoneFunction;
		std::function<void(void)> m_CancelFunction;

	};
}
