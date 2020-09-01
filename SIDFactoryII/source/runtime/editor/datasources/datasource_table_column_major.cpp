#include "datasource_table_column_major.h"
#include "runtime/emulation/cpumemory.h"

#include <assert.h>

namespace Editor
{
	DataSourceTableColumnMajor::DataSourceTableColumnMajor(Emulation::CPUMemory* inCPUMemory, unsigned short inSourceAddress, int inRowCount, int inColumnCount)
		: DataSourceTable(inCPUMemory, inSourceAddress, inRowCount, inColumnCount)
	{
		m_RawDataBuffer = new unsigned char[m_DataSize];

		PullDataFromSource();
	}

	DataSourceTableColumnMajor::~DataSourceTableColumnMajor()
	{
		delete[] m_RawDataBuffer;
	}

	//------------------------------------------------------------------------------------------------------------------

	bool DataSourceTableColumnMajor::PushDataToSource()
	{
		assert(m_CPUMemory != nullptr);
		assert(m_CPUMemory->IsLocked());
		assert(m_Data != nullptr);
		assert(m_RawDataBuffer != nullptr);

		int i = 0;

		for (unsigned int row = 0; row < m_RowCount; ++row)						// Number of things
		{
			for (unsigned int column = 0; column < m_ColumnCount; ++column)		// A cells of the thing
			{
				unsigned int write_index = column * m_RowCount + row;
				unsigned char data = m_Data[i];
				m_RawDataBuffer[write_index] = data;

				++i;
			}
		}

		m_CPUMemory->SetData(m_SourceAddress, m_RawDataBuffer, m_DataSize);

		return true;
	}

	//------------------------------------------------------------------------------------------------------------------

	void DataSourceTableColumnMajor::PullDataFromSource()
	{
		assert(m_CPUMemory != nullptr);
		assert(m_Data != nullptr);
		assert(m_RawDataBuffer != nullptr);

		m_CPUMemory->Lock();
		m_CPUMemory->GetData(m_SourceAddress, m_RawDataBuffer, m_DataSize);
		m_CPUMemory->Unlock();

		int i = 0;

		for (unsigned int row = 0; row < m_RowCount; ++row)						// Number of things
		{
			for (unsigned int column = 0; column < m_ColumnCount; ++column)		// A cells of the thing
			{
				unsigned int read_index = column * m_RowCount + row;
				unsigned char data = m_RawDataBuffer[read_index];
				m_Data[i] = data;

				++i;
			}
		}
	}
}