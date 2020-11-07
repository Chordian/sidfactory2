#include "datasource_table.h"
#include "runtime/emulation/cpumemory.h"
#include "foundation/base/assert.h"

namespace Editor
{
	DataSourceTable::DataSourceTable(Emulation::CPUMemory* inCPUMemory, unsigned short inSourceAddress, unsigned int inRowCount, unsigned int inColumnCount)
		: DataSourceEmulationMemory(inCPUMemory, inSourceAddress, inRowCount * inColumnCount)
		, m_RowCount(inRowCount)
		, m_ColumnCount(inColumnCount)
	{
	}

	DataSourceTable::~DataSourceTable()
	{
	}

	//------------------------------------------------------------------------------------------------------------------

	const unsigned char DataSourceTable::operator[](int inIndex) const
	{
		FOUNDATION_ASSERT(inIndex >= 0);
		FOUNDATION_ASSERT(inIndex < m_DataSize);

		return m_Data[inIndex];
	}


	unsigned char& DataSourceTable::operator[](int inIndex)
	{
		FOUNDATION_ASSERT(inIndex >= 0);
		FOUNDATION_ASSERT(inIndex < m_DataSize);

		return m_Data[inIndex];
	}

	//------------------------------------------------------------------------------------------------------------------

	const unsigned int DataSourceTable::GetRowCount() const
	{
		return m_RowCount;
	}

	const unsigned int DataSourceTable::GetColumnCount() const
	{
		return m_ColumnCount;
	}
}