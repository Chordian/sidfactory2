#pragma once
/*
#include "idatasource.h"
#include "datasource_memory_buffer.h"

namespace Emulation
{
	class CPUMemory;
}

namespace Editor
{
	class DataSourceMemoryBlock : public DataSourceMemoryBuffer
	{
	public:
		DataSourceMemoryBlock(Emulation::CPUMemory* inCPUMemory, unsigned short inSourceAddress, int inBlockSize);

		unsigned char& operator[](int inIndex);
		bool PushDataToSource() override;
		void PullDataFromSource();

	private:

		Emulation::CPUMemory* m_CPUMemory;
		unsigned short m_SourceAddress;
	};
}

*/