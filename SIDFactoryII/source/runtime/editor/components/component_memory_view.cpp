#include "component_memory_view.h"

#include "foundation/graphics/textfield.h"
#include "foundation/input/mouse.h"
#include "foundation/input/keyboard.h"
#include "foundation/input/keyboard_utils.h"

#include "runtime/editor/cursor_control.h"
#include "runtime/editor/debug/debug_singleton.h"
#include "runtime/editor/display_state.h"

#include "SDL_keycode.h"
#include <assert.h>

using namespace Foundation;

namespace Editor
{
	int ComponentMemoryView::GetWidthFromColumns(int inColumnCount)
	{
		// Example, 3 rows: "xxxx: 01 02 03"
		return 5 + 3 * inColumnCount;
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	ComponentMemoryView::ComponentMemoryView(int inID, int inGroupID, Undo* inUndo, std::shared_ptr<DataSourceTableMemoryView> inDataSource, Foundation::TextField* inTextField, int inX, int inY, int inHeight)
		: ComponentBase(inID, inGroupID, inUndo, inTextField, inX, inY, GetWidthFromColumns(inDataSource->GetColumnCount()), inHeight)
		, m_DataSource(inDataSource)
		, m_MemoryAddress(0x1000)
	{
		assert(inDataSource != nullptr);
		assert(inTextField != nullptr);
	}

	ComponentMemoryView::~ComponentMemoryView()
	{

	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	bool ComponentMemoryView::ConsumeInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		if (m_HasControl)
		{
			// Get key events
			for (const auto& key_event : inKeyboard.GetKeyEventList())
			{
				switch (key_event)
				{
				case SDLK_UP:
					m_MemoryAddress -= static_cast<unsigned short>(m_DataSource->GetColumnCount());
					return true;
				case SDLK_DOWN:
					m_MemoryAddress += static_cast<unsigned short>(m_DataSource->GetColumnCount());
					return true;
				case SDLK_PAGEUP:
					m_MemoryAddress -= static_cast<unsigned short>(m_DataSource->GetColumnCount() * 8);
					break;
				case SDLK_PAGEDOWN:
					m_MemoryAddress += static_cast<unsigned short>(m_DataSource->GetColumnCount() * 8);
					break;
				default:
					break;
				}
			}
		}

		return false;
	}


	bool ComponentMemoryView::ConsumeInput(const Foundation::Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		return false;
	}


	void ComponentMemoryView::ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse)
	{

	}


	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentMemoryView::Refresh(const DisplayState& inDisplayState)
	{
		if (m_TextField->IsEnabled())
		{
			if (DebugSingleton::GetInstance()->HasDebugAddress())
			{
				m_MemoryAddress = DebugSingleton::GetInstance()->ConsumeDebugAddress();
				m_RequireRefresh = true;
			}
		}

		if (m_RequireRefresh && m_TextField->IsEnabled())
		{
			const bool is_uppercase = inDisplayState.IsHexUppercase();

			// This refreshes the contents of the data source
			m_DataSource->SetAddress(m_MemoryAddress);

			int y = m_Position.m_Y;
			unsigned short address = m_MemoryAddress;

			for (unsigned int i = 0; i < static_cast<unsigned int>(m_Dimensions.m_Height); i++)
			{
				m_TextField->PrintHexValue(m_Position.m_X, y, is_uppercase, address);
				m_TextField->PrintChar(m_Position.m_X + 4, y, ':');

				unsigned int base_data_offset = i * m_DataSource->GetColumnCount();

				for (unsigned int j = 0; j < m_DataSource->GetColumnCount(); j++)
				{
					unsigned char value = (*m_DataSource)[base_data_offset + j];
					m_TextField->PrintHexValue(m_Position.m_X + 6 + (j * 3), y, is_uppercase, value);
				}

				address += static_cast<unsigned short>(m_DataSource->GetColumnCount());
				++y;
			}
		}
	}


	void ComponentMemoryView::HandleDataChange()
	{
		if (m_HasDataChange)
			m_HasDataChange = false;
	}


	void ComponentMemoryView::PullDataFromSource()
	{

	}


	void ComponentMemoryView::ExecuteInsertDeleteRule(const DriverInfo::TableInsertDeleteRule& inRule, int inSourceTableID, int inIndexPre, int inIndexPost)
	{

	}


	void ComponentMemoryView::ExecuteAction(int inActionInput)
	{

	}

}