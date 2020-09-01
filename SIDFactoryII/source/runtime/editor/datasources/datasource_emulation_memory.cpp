#include "datasource_emulation_memory.h"
#include "runtime/emulation/cpumemory.h"
#include <assert.h>

namespace Editor
{
	DataSourceEmulationMemory::DataSourceEmulationMemory(Emulation::CPUMemory* inCPUMemory, unsigned short inSourceAddress, int inBlockSize)
		: DataSourceMemory(inBlockSize)
		, m_CPUMemory(inCPUMemory)
		, m_SourceAddress(inSourceAddress)
	{
	}


	const unsigned short DataSourceEmulationMemory::GetSourceAddress() const
	{
		return m_SourceAddress;
	}
}