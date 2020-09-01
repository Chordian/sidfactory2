#pragma once

#include "datasource_table.h"

namespace Emulation
{
	class CPUMemory;
}

namespace Editor
{
	class DataSourceTableMemoryView final : public DataSourceTable
	{
	public:
		DataSourceTableMemoryView(Emulation::CPUMemory* inCPUMemory, unsigned short inSourceAddress, int inRowCount, int inColumnCount);
		~DataSourceTableMemoryView();

		bool PushDataToSource() override;
		void PullDataFromSource() override;

		void SetAddress(unsigned short inAddress);

	private:
	};
}