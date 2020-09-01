#pragma once

#include "datasource_table.h"

namespace Emulation
{
	class CPUMemory;
}

namespace Editor
{
	class DataSourceTableColumnMajor final : public DataSourceTable
	{
	public:
		DataSourceTableColumnMajor(Emulation::CPUMemory* inCPUMemory, unsigned short inSourceAddress, int inRowCount, int inColumnCount);
		~DataSourceTableColumnMajor();

		bool PushDataToSource() override;
		void PullDataFromSource() override;

	private:

		unsigned char* m_RawDataBuffer;
	};
}