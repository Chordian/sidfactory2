#pragma once

#include "datasource_emulation_memory.h"

namespace Emulation
{
	class CPUMemory;
}

namespace Editor
{
	class DataSourceTable : public DataSourceEmulationMemory
	{
	protected:
		DataSourceTable(Emulation::CPUMemory* inCPUMemory, unsigned short inSourceAddress, unsigned int inRowCount, unsigned int inColumnCount);

	public:
		virtual ~DataSourceTable();
		
		unsigned char& operator[](int inIndex);
		const unsigned char operator[](int inIndex) const;

		const unsigned int GetRowCount() const;
		const unsigned int GetColumnCount() const;

	protected:
		unsigned int m_RowCount;
		unsigned int m_ColumnCount;
	};
}
