#include "dialog_optimize.h"

#include "foundation/graphics/viewport.h"
#include "foundation/graphics/textfield.h"
#include "foundation/base/types.h"
#include "foundation/graphics/color.h"
#include "foundation/input/keyboard.h"
#include "foundation/input/mouse.h"
#include "runtime/editor/datasources/datasource_orderlist.h"
#include "runtime/editor/datasources/datasource_sequence.h"
#include "runtime/editor/datasources/datasource_table.h"
#include "runtime/editor/optimize/optimizer.h"
#include "runtime/editor/instrument/instrumentdata.h"
#include "runtime/editor/display_state.h"
#include "runtime/editor/components_manager.h"
#include "utils/usercolors.h"
#include <algorithm>

using namespace Foundation;
using namespace Utility;

namespace Editor
{
	DialogOptimize::DialogOptimize(
		std::vector<std::shared_ptr<DataSourceOrderList>> inOrderListDataSources,
		std::vector<std::shared_ptr<DataSourceSequence>> inSequenceDataSources,
		std::shared_ptr<DataSourceTable> inInstrumentTableDataSource,
		std::shared_ptr<DataSourceTable> inCommandTableDataSource,
		int inInstrumentsTableID,
		int inCommandsTableID,
		const ComponentsManager& inComponentsManager,
		DriverInfo& inDriverInfo,
		Emulation::CPUMemory* inCPUMemory,
		std::function<void()> inOnDoneCallback
	)
		: DialogBase()
		, m_Width(51)
		, m_Height(24)
		, m_OnDoneCallback(inOnDoneCallback)
	{
		m_Optimizer = std::make_unique<Optimizer>(inCPUMemory, inDriverInfo, inOrderListDataSources, inSequenceDataSources, inInstrumentTableDataSource, inCommandTableDataSource, inInstrumentsTableID, inCommandsTableID);
		m_InstrumentData = InstrumentData::Create(0, inDriverInfo, inComponentsManager);
	}


	DialogOptimize::~DialogOptimize() = default;


	void DialogOptimize::Cancel()
	{
		m_OnDoneCallback();
	}



	bool DialogOptimize::ConsumeInput(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse)
	{
		DialogBase::ConsumeInput(inKeyboard, inMouse);

		for (const auto& key_event : inKeyboard.GetKeyEventList())
		{
			switch (key_event)
			{
			case SDLK_ESCAPE:
				m_Done = true;
				return true;
			case SDLK_RETURN:
				if (!m_ComponentsManager->IsComponentInFocus(m_ComponentButtonOk->GetComponentID()))
					m_ComponentsManager->SetComponentInFocus(m_ComponentButtonOk);

				return true;
			}
		}

		return false;
	}


	void DialogOptimize::Refresh(const DisplayState& inDisplayState)
	{
		DialogBase::Refresh(inDisplayState);

		int x = 2;
		int y = 2;

		const bool is_uppercase = inDisplayState.IsHexUppercase();

		m_TextField->Print({ x, y }, ToColor(UserColor::DialogText), "Instruments:");
		y = PrintUsedNumbers(x, y + 1, is_uppercase, 0x20, 0x10, m_Optimizer->GetUsedInstrumentIndices());

		m_TextField->Print({ x, y }, ToColor(UserColor::DialogText), "Commands:");
		y = PrintUsedNumbers(x, y + 1, is_uppercase, 0x40, 0x10, m_Optimizer->GetUsedCommandIndices());

		m_TextField->Print({ x, y }, ToColor(UserColor::DialogText), "Sequences:");
		PrintUsedNumbers(x, y + 1, is_uppercase, 0x80, 0x10, m_Optimizer->GetUsedSequenceIndices());
	}


	int DialogOptimize::PrintUsedNumbers(int inX, int inY, bool inUppercase, unsigned char inNumberCount, unsigned char inNumbersPerRowCount, const std::vector<unsigned char> inUsedArray)
	{
		int x = inX;
		int y = inY;
		int counter = 0;

		for (unsigned char i = 0; i < inNumberCount; ++i)
		{
			Foundation::Color color = 
				ToColor(std::find(inUsedArray.begin(), inUsedArray.end(), i) != inUsedArray.end() ? UserColor::DialogOptimzerUsed : UserColor::DialogOptimzerUnused);
			m_TextField->PrintHexValue(x, y, color, inUppercase, i);

			++counter;

			if (counter >= inNumbersPerRowCount)
			{
				counter = 0;
				x = inX;
				++y;
			}
			else
				x += 3;
		}

		return y + 1;
	}


	void DialogOptimize::ActivateInternal(Foundation::Viewport* inViewport)
	{
		m_TextField = inViewport->CreateTextField(m_Width, m_Height, 0, 0);
		m_TextField->SetEnable(true);
		m_TextField->SetPositionToCenterOfViewport();

		m_TextField->ColorAreaBackground(ToColor(UserColor::DialogBackground));

		const std::string caption = "Optimize";

		m_TextField->ColorAreaBackground(ToColor(UserColor::DialogHeader), { {0, 0}, {m_Width, 1} });
		m_TextField->Print((m_Width - static_cast<int>(caption.length())) >> 1, 0, caption);

		m_ComponentsManager = std::make_unique<ComponentsManager>(inViewport, m_CursorControl);
		m_ComponentsManager->SetGroupEnabledForInput(0, true);

		const int button_width = 10;

		m_ComponentButtonOk = std::make_shared<ComponentButton>(0, 0,
			nullptr,
			m_TextField, "Ok",
			(m_Width - button_width) >> 1, m_Height - 2,
			button_width,
			[&]() { OnDone(); });
		m_ComponentsManager->AddComponent(m_ComponentButtonOk);

		m_ComponentsManager->SetComponentInFocus(m_ComponentButtonOk);
	}


	void DialogOptimize::DeactivateInternal(Foundation::Viewport* inViewport)
	{
		inViewport->Destroy(m_TextField);
	}


	void DialogOptimize::OnDone()
	{
		m_Optimizer->Execute();

		m_Done = true;
		m_OnDoneCallback();
	}

}
