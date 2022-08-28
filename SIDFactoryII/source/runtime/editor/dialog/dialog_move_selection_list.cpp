#include "dialog_move_selection_list.h"

#include "foundation/graphics/viewport.h"
#include "foundation/graphics/textfield.h"
#include "foundation/base/types.h"
#include "foundation/graphics/color.h"
#include "foundation/input/keyboard.h"
#include "foundation/input/mouse.h"
#include "runtime/editor/components_manager.h"
#include "utils/usercolors.h"

using namespace Foundation;
using namespace Utility;

namespace Editor
{
	DialogMoveSelectionList::DialogMoveSelectionList(
		int inWidth,
		int inHeight,
		int inSelectionIndex,
		const std::string& inCaption,
		const std::vector<std::string>& inSelectionList,
		std::function<void(const unsigned int, const unsigned int)>&& inMoveSelect,
		std::function<void(void)>&& inCancel
	)
		: DialogSelectionList(inWidth, inHeight, inSelectionIndex, inCaption, inSelectionList, std::move(std::function<void(const unsigned int)>()), std::move(inCancel))
		, m_SelectedItemToMove(false)
		, m_MoveSelectFunction(inMoveSelect)
		, m_SelectionItemToMoveIndex(0)
	{

	}


	bool DialogMoveSelectionList::ConsumeInput(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse)
	{
		const unsigned int previous_selection_index = static_cast<unsigned int>(m_StringListSelectorComponent->GetSelectionIndex());
		DialogBase::ConsumeInput(inKeyboard, inMouse);
		const unsigned int selection_index = static_cast<unsigned int>(m_StringListSelectorComponent->GetSelectionIndex());

		if (m_SelectedItemToMove && selection_index != previous_selection_index)
		{
			std::string previous = (*m_StringListDataBuffer)[previous_selection_index];
			(*m_StringListDataBuffer)[previous_selection_index] = (*m_StringListDataBuffer)[selection_index];
			(*m_StringListDataBuffer)[selection_index] = previous;
		}

		for (const auto& key_event : inKeyboard.GetKeyEventList())
		{
			switch (key_event)
			{
			case SDLK_ESCAPE:
				m_Done = true;
				m_CancelFunction();

				return true;
			case SDLK_RETURN:
				if (!m_SelectedItemToMove)
				{
					m_SelectedItemToMove = true;
					m_SelectionItemToMoveIndex = selection_index;
				}
				else
				{
					m_Done = true;
					m_MoveSelectFunction(m_SelectionItemToMoveIndex, selection_index);
				}
				return true;
			default:
				break;
			}
		}

		return false;
	}
}