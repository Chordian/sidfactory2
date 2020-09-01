#include "datasource_memory_buffer_string.h"

namespace Editor
{
	DataSourceMemoryBufferString::DataSourceMemoryBufferString(int inBlockSize)
		: DataSourceMemoryBuffer(inBlockSize)
	{

	}

	void DataSourceMemoryBufferString::operator = (const std::string& inString)
	{
		const int string_length = static_cast<int>(inString.length());
		const int data_length = string_length < m_DataSize ? string_length : m_DataSize;

		const char* string_data = inString.c_str();

		for (int i = 0; i < data_length; ++i)
			m_Data[i] = static_cast<unsigned char>(string_data[i]);

		if (data_length < m_DataSize)
			m_Data[data_length] = 0;
	}

	std::string DataSourceMemoryBufferString::GetAsString() const
	{
		std::string output;

		for (int i = 0; i < m_DataSize; ++i)
		{
			if (m_Data[i] == '\0')
				break;

			output += static_cast<char>(m_Data[i]);
		}

		return output;
	}
}
