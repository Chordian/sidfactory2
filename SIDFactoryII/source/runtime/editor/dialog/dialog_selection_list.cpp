#include "dialog_selection_list.h"

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
	DialogSelectionList::DialogSelectionList(
		int inWidth,
		int inHeight,
		const std::string& inCaption,
		const std::vector<std::string>& inSelectionList,
		std::function<void(const unsigned int)>&& inSelect,
		std::function<void(void)>&& inCancel
	)
		: DialogBase()
		, m_Width(inWidth)
		, m_Height(inHeight)
		, m_Caption(inCaption)
		, m_SelectionList(inSelectionList)
		, m_SelectFunction(inSelect)
		, m_CancelFunction(inCancel)
	{

	}


	void DialogSelectionList::Cancel()
	{
		if (m_CancelFunction)
			m_CancelFunction();
	}


	bool DialogSelectionList::ConsumeInput(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse)
	{
		DialogBase::ConsumeInput(inKeyboard, inMouse);

		for (const auto& key_event : inKeyboard.GetKeyEventList())
		{
			switch (key_event)
			{
			case SDLK_ESCAPE:
				m_Done = true;
				m_CancelFunction();

				return true;
			case SDLK_RETURN:
			{
				m_Done = true;
				const unsigned int selected = static_cast<unsigned int>(m_StringListSelectorComponent->GetSelectionIndex());
				m_SelectFunction(selected);
			}
			return true;
			}
		}

		return false;
	}


	void DialogSelectionList::ActivateInternal(Foundation::Viewport* inViewport)
	{
		m_TextField = inViewport->CreateTextField(m_Width, m_Height, 0, 0);
		m_TextField->SetEnable(true);
		m_TextField->SetPositionToCenterOfViewport();

		m_TextField->ColorAreaBackground(ToColor(UserColor::DialogBackground));

		m_TextField->ColorAreaBackground(ToColor(UserColor::DialogHeader), { {0, 0}, {m_Width, 1} });
		m_TextField->Print((m_Width - static_cast<int>(m_Caption.length())) >> 1, 0, ToColor(UserColor::DialogHeaderText), m_Caption);

		m_ComponentsManager = std::make_unique<ComponentsManager>(inViewport, m_CursorControl);
		m_ComponentsManager->SetGroupEnabledForInput(0, true);

		m_StringListDataBuffer = std::make_shared<DataSourceTList<std::string>>(std::vector<std::string>(m_SelectionList));
		m_StringListSelectorComponent = std::make_shared<ComponentStringListSelector>
		(
			0, 0,
			nullptr,
			m_StringListDataBuffer,
			m_TextField,
			1, 1,
			m_Width - 2, m_Height,
			1, 1
		);

		m_StringListSelectorComponent->SetColors(ToColor(UserColor::DialogBackground), ToColor(UserColor::DialogListSelectorCursorFocus), ToColor(UserColor::DialogListSelectorCursor));
		m_StringListSelectorComponent->SetColors(ToColor(UserColor::DialogText));

		m_ComponentsManager->AddComponent(m_StringListSelectorComponent);
		m_ComponentsManager->SetComponentInFocus(m_StringListSelectorComponent);
	}


	void DialogSelectionList::DeactivateInternal(Foundation::Viewport* inViewport)
	{
		inViewport->Destroy(m_TextField);
	}
}