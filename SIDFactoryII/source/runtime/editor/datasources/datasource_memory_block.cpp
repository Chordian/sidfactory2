/*
#include "datasource_memory_block.h"
#include "runtime/emulation/cpumemory.h"
#include <assert.h>

namespace Editor
{
	DataSourceMemoryBlock::DataSourceMemoryBlock(Emulation::CPUMemory* inCPUMemory, unsigned short inSourceAddress, int inBlockSize)
		: DataSourceMemoryBuffer(inBlockSize)
		, m_CPUMemory(inCPUMemory)
		, m_SourceAddress(inSourceAddress)
	{
		PullDataFromSource();
	}

	//------------------------------------------------------------------------------------------------------------------

	unsigned char& DataSourceMemoryBlock::operator[](int inIndex)
	{
		assert(inIndex >= 0);
		assert(inIndex < m_DataSize);

		return m_Data[inIndex];
	}

	//------------------------------------------------------------------------------------------------------------------

	bool DataSourceMemoryBlock::PushDataToSource()
	{
		assert(m_CPUMemory != nullptr);
		assert(m_CPUMemory->IsLocked());
		assert(m_Data != nullptr);

		m_CPUMemory->SetData(m_SourceAddress, m_Data, m_DataSize);

		return true;
	}

	//------------------------------------------------------------------------------------------------------------------

	void DataSourceMemoryBlock::PullDataFromSource()
	{
		assert(m_CPUMemory != nullptr);
		assert(m_Data != nullptr);

		m_CPUMemory->Lock();
		m_CPUMemory->GetData(m_SourceAddress, m_Data, m_DataSize);
		m_CPUMemory->Unlock();
	}
}
*/