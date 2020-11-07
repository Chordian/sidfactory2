#include "datasource_memory_buffer.h"
#include "runtime/emulation/cpumemory.h"
#include <cstring>
#include "foundation/base/assert.h"

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
		FOUNDATION_ASSERT(inIndex >= 0);
		FOUNDATION_ASSERT(inIndex < m_DataSize);

		return m_Data[inIndex];
	}

	//------------------------------------------------------------------------------------------------------------------

	bool DataSourceMemoryBuffer::PushDataToSource()
	{
		return true;
	}
}
