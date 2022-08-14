#pragma once

#include "screen_base.h"
#include "runtime/editor/driver/driver_info.h"
#include "runtime/editor/driver/driver_state.h"
#include "runtime/emulation/imemoryrandomreadaccess.h"
#include <memory>
#include <vector>

namespace Emulation
{
	class CPUMemory;
}

namespace Editor
{
	class DataSourceOrderList;
	class DataSourceSequence;

	namespace ScreenEditUtils
	{
		void PrepareSequenceData(const Editor::DriverInfo& inDriverInfo, Emulation::CPUMemory& inCPUMemory);
		void PrepareSequencePointers(const Editor::DriverInfo& inDriverInfo, Emulation::CPUMemory& inCPUMemory);
		void PrepareAllOrderListsDataSources(const Editor::DriverInfo& inDriverInfo, Emulation::CPUMemory& inCPUMemory, std::vector<std::shared_ptr<DataSourceOrderList>>& outOrderListDataSources);
		void PrepareNotSelectedSongOrderListsDataSources(const Editor::DriverInfo& inDriverInfo, Emulation::CPUMemory& inCPUMemory, std::vector<std::shared_ptr<DataSourceOrderList>>& outOrderListDataSources);
		void PrepareSelectedSongOrderListsDataSources(const Editor::DriverInfo& inDriverInfo, Emulation::CPUMemory& inCPUMemory, std::vector<std::shared_ptr<DataSourceOrderList>>& outOrderListDataSources);
		void PrepareSequenceDataSources(const Editor::DriverInfo& inDriverInfo, const Editor::DriverState& inDriverState, Emulation::CPUMemory& inCPUMemory, std::vector<std::shared_ptr<DataSourceSequence>>& outSequenceDataSources);
	};
}