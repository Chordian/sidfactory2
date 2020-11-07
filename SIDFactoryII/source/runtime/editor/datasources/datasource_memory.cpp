#include "datasource_memory.h"
#include "runtime/emulation/cpumemory.h"
#include <cstring>
#include "foundation/base/assert.h"

namespace Editor
{
	DataSourceMemory::DataSourceMemory()
		: IDataSource()
	{

	}

	DataSourceMemory::DataSourceMemory(int inBlockSize)
		: IDataSource()
		, m_DataSize(inBlockSize)
	{
		m_Data = new unsigned char[m_DataSize];
		memset(m_Data, 0, m_DataSize);
	}

	DataSourceMemory::~DataSourceMemory()
	{
		delete m_Data;
	}

	//------------------------------------------------------------------------------------------------------------------

	const int DataSourceMemory::GetSize() const
	{
		return m_DataSize;
	}
}
