#include "dialog_usbsid_boards.h"

#include "foundation/input/keyboard.h"
#include "foundation/input/mouse.h"

namespace Editor
{
	DialogUSBSIDSelection::DialogUSBSIDSelection(
		int inWidth,
		const std::string& inCaption,
		const std::vector<std::string>& inLabels,
		const std::vector<bool>& inMarked,
		std::function<void(const std::vector<bool>&)>&& inSelect,
		std::function<void(void)>&& inCancel
	)
		: DialogSelectionList(
			inWidth,
			static_cast<int>(inLabels.size()) + 3,
			0,
			inCaption,
			MakeLines(inLabels, inMarked),
			[this](const unsigned int inCursorIndex) { Confirm(inCursorIndex); },
			std::move(inCancel))
		, m_Labels(inLabels)
		, m_Marked(inMarked)
		, m_SelectRowsFunction(inSelect)
	{
		m_Marked.resize(m_Labels.size(), false);
	}


	bool DialogUSBSIDSelection::ConsumeInput(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse)
	{
		for (const auto& key_event : inKeyboard.GetKeyEventList())
		{
			if (key_event == SDLK_SPACE)
			{
				const unsigned int index = static_cast<unsigned int>(m_StringListSelectorComponent->GetSelectionIndex());

				if (index < m_Marked.size())
				{
					m_Marked[index] = !m_Marked[index];
					(*m_StringListDataBuffer)[static_cast<int>(index)] = MakeLine(m_Labels[index], m_Marked[index]);
					m_StringListSelectorComponent->ForceRefresh();
				}

				return true;
			}
		}

		return DialogSelectionList::ConsumeInput(inKeyboard, inMouse);
	}


	std::vector<std::string> DialogUSBSIDSelection::MakeLines(const std::vector<std::string>& inLabels, const std::vector<bool>& inMarked)
	{
		std::vector<std::string> lines;

		for (size_t i = 0; i < inLabels.size(); ++i)
			lines.push_back(MakeLine(inLabels[i], i < inMarked.size() && inMarked[i]));

		return lines;
	}


	std::string DialogUSBSIDSelection::MakeLine(const std::string& inLabel, bool inMarked)
	{
		return std::string(inMarked ? "[x] " : "[ ] ") + inLabel;
	}


	void DialogUSBSIDSelection::Confirm(unsigned int inCursorIndex)
	{
		std::vector<bool> rows = m_Marked;
		bool any_marked = false;

		for (const bool marked : rows)
			any_marked |= marked;

		if (!any_marked && inCursorIndex < rows.size())
			rows[inCursorIndex] = true;

		if (m_SelectRowsFunction)
			m_SelectRowsFunction(rows);
	}
}
