#pragma once

#include "datasource_table.h"

namespace Emulation
{
	class CPUMemory;
}

namespace Editor
{
	class DataSourceTableRowMajor final : public DataSourceTable
	{
	public:
		DataSourceTableRowMajor(Emulation::CPUMemory* inCPUMemory, unsigned short inSourceAddress, int inRowCount, int inColumnCount);
		~DataSourceTableRowMajor();

		bool PushDataToSource() override;
		void PullDataFromSource() override;

	private:
	};
}