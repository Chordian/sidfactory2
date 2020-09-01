#pragma once

#include "idatasource.h"
#include "datasource_memory.h"

namespace Emulation
{
	class CPUMemory;
}

namespace Editor
{
	class DataSourceEmulationMemory : public DataSourceMemory
	{
	public:
		DataSourceEmulationMemory(Emulation::CPUMemory* inCPUMemory, unsigned short inSourceAddress, int inBlockSize);

		const unsigned short GetSourceAddress() const;
		virtual void PullDataFromSource() = 0;

	protected:
		Emulation::CPUMemory* m_CPUMemory;
		unsigned short m_SourceAddress;
	};
}
