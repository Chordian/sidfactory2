#include "datasource_table_memory_view.h"
#include "runtime/emulation/cpumemory.h"

#include "foundation/base/assert.h"

namespace Editor
{
	DataSourceTableMemoryView::DataSourceTableMemoryView(Emulation::CPUMemory* inCPUMemory, unsigned short inSourceAddress, int inRowCount, int inColumnCount)
		: DataSourceTable(inCPUMemory, inSourceAddress, inRowCount, inColumnCount)
	{
		PullDataFromSource();
	}

	DataSourceTableMemoryView::~DataSourceTableMemoryView()
	{
	}

	//------------------------------------------------------------------------------------------------------------------

	bool DataSourceTableMemoryView::PushDataToSource()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------------------------

	void DataSourceTableMemoryView::SetAddress(unsigned short inAddress)
	{
		m_SourceAddress = inAddress;
		PullDataFromSource();
	}

	void DataSourceTableMemoryView::PullDataFromSource()
	{
		FOUNDATION_ASSERT(m_CPUMemory != nullptr);
		FOUNDATION_ASSERT(m_Data != nullptr);

		unsigned int end_address = static_cast<unsigned int>(m_SourceAddress) + static_cast<unsigned int>(m_DataSize);

		m_CPUMemory->Lock();

		if (end_address <= 0x10000)
			m_CPUMemory->GetData(m_SourceAddress, m_Data, m_DataSize);
		else
		{
			unsigned short first_address = m_SourceAddress;
			unsigned short first_size = static_cast<unsigned short>(0x10000 - m_SourceAddress);
			unsigned short second_address = 0x0000;
			unsigned short second_size = static_cast<unsigned short>(m_DataSize - first_size);

			m_CPUMemory->GetData(first_address, m_Data, first_size);
			m_CPUMemory->GetData(second_address, &m_Data[first_size], second_size);
		}

		m_CPUMemory->Unlock();
	}
}