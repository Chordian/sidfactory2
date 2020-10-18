#include "component_string_list_selector.h"
#include "runtime/editor/cursor_control.h"
#include "foundation/input/mouse.h"
#include "foundation/input/keyboard.h"
#include "foundation/input/keyboard_utils.h"
#include "foundation/graphics/textfield.h"

using namespace Foundation;

namespace Editor
{
	ComponentStringListSelector::ComponentStringListSelector(
		int inID, 
		int inGroupID, 
		Undo* inUndo, 
		std::shared_ptr<DataSourceTList<std::string>> inDataSource, 
		Foundation::TextField* inTextField, 
		int inX, 
		int inY, 
		int inWidth,
		int inHeight, 
		int inHorizontalMargin, 
		int inVerticalMargin)
		: ComponentListSelector(inID, inGroupID, inUndo, inDataSource, inTextField, inX, inY, inWidth, inHeight, inHorizontalMargin, inVerticalMargin)
		, m_DataSourceStringList(inDataSource)
	{
	}

	ComponentStringListSelector::~ComponentStringListSelector()
	{

	}

	void ComponentStringListSelector::SetSelectionCallback(const std::function<void(bool)>& inOnSelectionCallback)
	{
		m_OnSelection = inOnSelectionCallback;
	}


	bool ComponentStringListSelector::ConsumeInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		if (m_HasControl)
		{
			if (ComponentListSelector::ConsumeInput(inKeyboard, inCursorControl, inComponentsManager))
				return true;

			for (auto& key_event : inKeyboard.GetKeyEventList())
			{
				switch (key_event)
				{
				case SDLK_RETURN:
					if (m_OnSelection)
						m_OnSelection(inKeyboard.IsModifierDownExclusive(Keyboard::Control));

					return true;
				}
			}
		}

		return false;
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	bool ComponentStringListSelector::ConsumeInput(const Foundation::Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		if (inMouse.IsButtonPressed(Mouse::Left))
		{
			Foundation::Point local_cell_position = m_TextField->GetCellPositionFromPixelPosition(inMouse.GetPosition()) - m_Position;

			if (local_cell_position.m_X >= m_VerticalMargin && local_cell_position.m_X < m_Dimensions.m_Width - m_VerticalMargin && local_cell_position.m_Y >= 0 && local_cell_position.m_Y < m_Dimensions.m_Height)
			{
				int cursor_pos = m_TopVisibleIndex + local_cell_position.m_Y - m_VerticalMargin;

				if (cursor_pos >= 0 && cursor_pos < m_DataSource->GetSize())
				{
					m_CursorPos = cursor_pos;
					m_RequireRefresh = true;

					if (m_OnSelection)
						m_OnSelection(inMouse.IsButtonDoublePressed(Mouse::Left));
				}

				return true;
			}
		}

		DoMouseWheel(inMouse);

		return false;
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentStringListSelector::RefreshLine(int inIndex, int inPosY)
	{
		std::string name = (*m_DataSourceStringList)[inIndex];

		if (static_cast<int>(name.length()) > m_ContentWidth)
			name = CondenseString(name, "...", m_ContentWidth);

		m_TextField->Print(m_ContentX, inPosY, m_TextColor, name);
	}
}

