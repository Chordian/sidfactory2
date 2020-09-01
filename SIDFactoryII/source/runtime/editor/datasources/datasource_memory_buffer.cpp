#include "datasource_memory_buffer.h"
#include "runtime/emulation/cpumemory.h"
#include <cstring>
#include <assert.h>

namespace Editor
{
	DataSourceMemoryBuffer::DataSourceMemoryBuffer(int inBlockSize)
		: DataSourceMemory(inBlockSize)
	{
		memset(m_Data, 0, inBlockSize);
	}

	//------------------------------------------------------------------------------------------------------------------

	unsigned char& DataSourceMemoryBuffer::operator[](int inIndex)
	{
		assert(inIndex >= 0);
		assert(inIndex < m_DataSize);

		return m_Data[inIndex];
	}

	//------------------------------------------------------------------------------------------------------------------

	bool DataSourceMemoryBuffer::PushDataToSource()
	{
		return true;
	}
}
