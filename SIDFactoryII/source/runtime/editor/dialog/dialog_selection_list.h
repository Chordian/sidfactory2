#pragma once

#include "dialog_base.h"
#include "runtime/editor/datasources/datasource_tlist.h"
#include "runtime/editor/components/component_string_list_selector.h"

namespace Editor
{
	class DialogSelectionList : public DialogBase
	{
	public:
		DialogSelectionList(
			int inWidth, 
			int inHeight, 
			int inSelectionIndex,
			const std::string& inCaption,
			const std::vector<std::string>& inSelectionList, 
			std::function<void(const unsigned int)>&& inSelect, 
			std::function<void(void)>&& inCancel
		);

		void Cancel() override;
		bool ConsumeInput(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse) override;

	protected:
		virtual void ActivateInternal(Foundation::Viewport* inViewport) override;
		virtual void DeactivateInternal(Foundation::Viewport* inViewport) override;

		const int m_Width;
		const int m_Height;

		const int m_InitialSelectionIndex;

		Foundation::TextField* m_TextField;

		const std::string m_Caption;
		const std::vector<std::string> m_SelectionList;
		
		const std::function<void(const unsigned int)> m_SelectFunction;
		const std::function<void(void)> m_CancelFunction;

		std::shared_ptr<DataSourceTList<std::string>> m_StringListDataBuffer;
		std::shared_ptr<ComponentStringListSelector> m_StringListSelectorComponent;
	};
}