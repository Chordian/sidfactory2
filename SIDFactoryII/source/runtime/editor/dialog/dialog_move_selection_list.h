#pragma once

#include "dialog_selection_list.h"

namespace Editor
{
	class DialogMoveSelectionList : public DialogSelectionList
	{
	public:
		DialogMoveSelectionList(
			int inWidth, 
			int inHeight, 
			int inSelectionIndex,
			const std::string& inCaption,
			const std::vector<std::string>& inSelectionList, 
			std::function<void(const unsigned int, const unsigned int)>&& inSelect, 
			std::function<void(void)>&& inCancel
		);

		bool ConsumeInput(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse) override;


	private:
		bool m_SelectedItemToMove;
		unsigned int m_SelectionItemToMoveIndex;

		const std::function<void(const unsigned int, const unsigned int)> m_MoveSelectFunction;
	};
}