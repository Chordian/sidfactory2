#pragma once

#include "datasource_memory_buffer.h"
#include <string>

namespace Editor
{
	class DataSourceMemoryBufferString : public DataSourceMemoryBuffer
	{
	public:
		DataSourceMemoryBufferString(int inBlockSize);

		void operator = (const std::string& inString);
		std::string GetAsString() const;
	};
}
