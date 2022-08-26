#pragma once

#include "dialog_base.h"
#include "runtime/editor/datasources/datasource_tlist.h"
#include "runtime/editor/components/component_string_list_selector.h"

namespace Editor
{
	class DialogUtilities : public DialogBase
	{
	public:
		enum class Selection : int
		{
			Statistics,
			Optimize,
			Pack,
			ClearSequences,
			ExpandSequences
		};

		DialogUtilities(int inWidth, int inHeight, std::function<void(const Selection)>&& inSelect, std::function<void(void)>&& inCancel);

		void Cancel() override;
		bool ConsumeInput(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse) override;

	protected:
		virtual void ActivateInternal(Foundation::Viewport* inViewport) override;
		virtual void DeactivateInternal(Foundation::Viewport* inViewport) override;

	private:
		const int m_Width;
		const int m_Height;

		Foundation::TextField* m_TextField;

		std::function<void(const Selection)> m_SelectFunction;
		std::function<void(void)> m_CancelFunction;

		std::shared_ptr<DataSourceTList<std::string>> m_StringListDataBuffer;
		std::shared_ptr<ComponentStringListSelector> m_StringListSelectorComponent;
	};
}