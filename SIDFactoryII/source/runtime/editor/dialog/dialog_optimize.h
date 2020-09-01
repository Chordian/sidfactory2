#pragma once

#include "dialog_base.h"
#include "runtime/editor/components/component_button.h"

#include <memory>
#include <functional>
#include <vector>

namespace Emulation
{
	class CPUMemory;
}

namespace Editor
{
	class DataSourceOrderList;
	class DataSourceSequence;
	class DataSourceTable;

	class Optimizer;
	class InstrumentData;

	class DialogOptimize : public DialogBase
	{
	public:
		enum class Selection : int
		{
			Statistics,
			Optimize,
			Pack
		};

		DialogOptimize(
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
		);

		~DialogOptimize();

		void Cancel() override;

		bool ConsumeInput(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse) override;
		void Refresh(const DisplayState& inDisplayState) override;

	protected:
		virtual void ActivateInternal(Foundation::Viewport* inViewport) override;
		virtual void DeactivateInternal(Foundation::Viewport* inViewport) override;

	private:
		int PrintUsedNumbers(int inX, int inY, bool inUppercase, unsigned char inNumberCount, unsigned char inNumbersPerRowCount, const std::vector<unsigned char> inUsedArray);

		void OnDone();

		const int m_Width;
		const int m_Height;

		Foundation::TextField* m_TextField;

		std::function<void()> m_OnDoneCallback;

		std::unique_ptr<Optimizer> m_Optimizer;
		std::shared_ptr<InstrumentData> m_InstrumentData;
		std::shared_ptr<ComponentButton> m_ComponentButtonOk;
	};
}