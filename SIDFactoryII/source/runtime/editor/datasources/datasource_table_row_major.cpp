#include "datasource_table_row_major.h"
#include "runtime/emulation/cpumemory.h"

#include <assert.h>

namespace Editor
{
	DataSourceTableRowMajor::DataSourceTableRowMajor(Emulation::CPUMemory* inCPUMemory, unsigned short inSourceAddress, int inRowCount, int inColumnCount)
		: DataSourceTable(inCPUMemory, inSourceAddress, inRowCount, inColumnCount)
	{
		PullDataFromSource();
	}

	DataSourceTableRowMajor::~DataSourceTableRowMajor()
	{
	}

	//------------------------------------------------------------------------------------------------------------------

	bool DataSourceTableRowMajor::PushDataToSource()
	{
		assert(m_CPUMemory != nullptr);
		assert(m_CPUMemory->IsLocked());
		assert(m_Data != nullptr);

		m_CPUMemory->SetData(m_SourceAddress, m_Data, m_DataSize);

		return true;
	}

	//------------------------------------------------------------------------------------------------------------------

	void DataSourceTableRowMajor::PullDataFromSource()
	{
		assert(m_CPUMemory != nullptr);
		assert(m_Data != nullptr);

		m_CPUMemory->Lock();
		m_CPUMemory->GetData(m_SourceAddress, m_Data, m_DataSize);
		m_CPUMemory->Unlock();
	}
}