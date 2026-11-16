#pragma once

#include "dialog_selection_list.h"

#include <functional>
#include <string>
#include <vector>

namespace Editor
{
	// List with multi selection: SPACE marks a row, ENTER confirms.
	// With no row marked, ENTER selects the row under the cursor.
	// Used for the USBSID-Pico board list and SID list.
	class DialogUSBSIDSelection final : public DialogSelectionList
	{
	public:
		DialogUSBSIDSelection(
			int inWidth,
			const std::string& inCaption,
			const std::vector<std::string>& inLabels,
			const std::vector<bool>& inMarked,
			std::function<void(const std::vector<bool>&)>&& inSelect,
			std::function<void(void)>&& inCancel
		);

		bool ConsumeInput(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse) override;

	private:
		static std::vector<std::string> MakeLines(const std::vector<std::string>& inLabels, const std::vector<bool>& inMarked);
		static std::string MakeLine(const std::string& inLabel, bool inMarked);

		void Confirm(unsigned int inCursorIndex);

		const std::vector<std::string> m_Labels;
		std::vector<bool> m_Marked;

		const std::function<void(const std::vector<bool>&)> m_SelectRowsFunction;
	};
}
