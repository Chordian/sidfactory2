#include "debug_views.h"

#include "foundation/graphics/viewport.h"
#include "foundation/graphics/textfield.h"
#include "foundation/input/keyboard.h"
#include "foundation/input/mouse.h"

#include "runtime/editor/components_manager.h"
#include "runtime/editor/components/component_memory_view.h"
#include "runtime/editor/datasources/datasource_table_memory_view.h"
#include "runtime/editor/driver/driver_info.h"
#include "runtime/emulation/cpumemory.h"

#include "utils/keyhook.h"

using namespace Foundation;
using namespace Emulation;
using namespace Utility;

namespace Editor
{
	const int DebugViews::ComponentBaseID = 0x8000;
	const int DebugViews::ComponentGroupID = 8;

	DebugViews::DebugViews(Viewport* inViewport, ComponentsManager* inComponentsManager, CPUMemory* inCPUMemory, const Foundation::Extent& inMainTextFieldDimensions, std::shared_ptr<const DriverInfo> inDriverInfo)
		: m_Enabled(false)
		, m_CPUMemory(inCPUMemory)
		, m_Viewport(inViewport)
		, m_ComponentsManager(inComponentsManager)
		, m_DriverInfo(inDriverInfo)
		, m_MemoryAddress(0x1000)
		, m_EventPos(-1)
	{
		const int view_width = 40;
		const int view_height = 30;

		m_TextField = m_Viewport->CreateTextField(view_width, view_height, 8 * (inMainTextFieldDimensions.m_Width - view_width - 1), 2 * 16);
		m_TextField->SetEnable(false);

		m_TextField->ColorAreaBackground(Color::DarkBlue);

		CreateViews(inComponentsManager);

		// Create key hooks for testing
		// m_KeyHookTests.push_back({ "Test.1", SDLK_a, Keyboard::Shift, [&]() { m_KeyHookTestValues[0]++; return true; } });
		// m_KeyHookTestValues.push_back(0);
		// m_KeyHookTests.push_back({ "Test.2", SDLK_a, Keyboard::Control, [&]() { m_KeyHookTestValues[1]++; return true; } });
		// m_KeyHookTestValues.push_back(0);
		// m_KeyHookTests.push_back({ "Test.3", SDLK_a, Keyboard::Control | Keyboard::Shift, [&]() { m_KeyHookTestValues[2]++; return true; } });
		// m_KeyHookTestValues.push_back(0);
	}

	DebugViews::~DebugViews()
	{
		m_Viewport->Destroy(m_TextField);
	}


	void DebugViews::SetEnabled(bool inEnabled)
	{
		if (inEnabled != m_Enabled)
		{
			m_Enabled = inEnabled;
			m_TextField->SetEnable(inEnabled);
			m_ComponentsManager->SetGroupEnabledForInput(ComponentGroupID, true);
			// m_ComponentsManager->SetGroupEnabledForTabbing(ComponentGroupID);
		}
	}


	bool DebugViews::IsEnabled() const
	{
		return m_Enabled;
	}


	void DebugViews::SetMemoryAddress(unsigned short inMemoryAddress)
	{
		m_MemoryAddress = inMemoryAddress;
	}


	void DebugViews::SetEventPosition(int inEventPos)
	{
		m_EventPos = inEventPos;
	}


	void DebugViews::Update(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse)
	{
		if (m_Enabled)
		{
			m_TextField->Print({ 1, 20 }, "Orderlist indices: ");
			m_TextField->Print({ 1, 21 }, "Sequence indices : ");
			m_TextField->Print({ 1, 22 }, "Sequence cursor  : ");
			m_TextField->Print({ 1, 23 }, "Tick counter     : ");

			const DriverInfo::DriverCommon& driver_common = m_DriverInfo->GetDriverCommon();

			m_CPUMemory->Lock();

			for (int i = 0; i < 3; ++i)
			{
				unsigned char orderlist_index = m_CPUMemory->GetByte(driver_common.m_OrderListIndexAddress + i);
				unsigned char sequence_index = m_CPUMemory->GetByte(driver_common.m_CurrentSequenceAddress + i);
				unsigned char sequence_cursor = m_CPUMemory->GetByte(driver_common.m_SequenceIndexAddress + i);
				unsigned char tick_counter = m_CPUMemory->GetByte(driver_common.m_TickCounterAddress + i);

				int x = 20 + (i * 3);

				m_TextField->PrintHexValue(x, 20, false, orderlist_index);
				m_TextField->PrintHexValue(x, 21, false, sequence_index);
				m_TextField->PrintHexValue(x, 22, false, sequence_cursor);
				m_TextField->PrintHexValue(x, 23, false, tick_counter);
			}

			m_CPUMemory->Unlock();

			{
				std::string key_string = "";

				for (auto& key_event : inKeyboard.GetKeyEventList())
					key_string += std::to_string(static_cast<int>(key_event)) + " ";

				m_TextField->Print({ 1, 24 }, "* " + key_string);
			}
			{
				//for (auto& key_event : inKeyboard.GetKeyEventList())
				//	Utility::ConsumeInputKeyHooks(key_event, inKeyboard.GetModiferMask(), m_KeyHookTests);

				std::string key_string = "";

				for (int& value : m_KeyHookTestValues)
					key_string += std::to_string(value) + " ";

				m_TextField->Print({ 1, 26 }, "* " + key_string);
			}

			m_TextField->Print({ 1, 27 }, inMouse.IsButtonDown(Mouse::Button::Left) ? "(o)" : "(*)");
			m_TextField->Print({ 4, 27 }, inMouse.IsButtonDown(Mouse::Button::Middle) ? "(o)" : "(*)");
			m_TextField->Print({ 7, 27 }, inMouse.IsButtonDown(Mouse::Button::Right) ? "(o)" : "(*)");

			m_TextField->Print({ 1, 29 }, "  EventPos: " + std::to_string(m_EventPos) + "     ");
			if ((m_EventPos & 7) == 0)
				m_TextField->Print({ 1, 29 }, "*");
		}
	}


	void DebugViews::CreateViews(ComponentsManager* inComponentsManager)
	{
		m_ComponentMemoryView = std::make_shared<ComponentMemoryView>
			(
				ComponentBaseID, 
				ComponentGroupID,
				nullptr,
				std::make_shared<DataSourceTableMemoryView>(m_CPUMemory, m_MemoryAddress, 18, 8),
				m_TextField,
				1, 1,
				18
			);

		inComponentsManager->AddComponent(m_ComponentMemoryView);
	}
}