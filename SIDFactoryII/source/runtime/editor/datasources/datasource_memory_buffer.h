#pragma once

#include "datasource_memory.h"

namespace Editor
{
	class DataSourceMemoryBuffer : public DataSourceMemory
	{
	public:
		DataSourceMemoryBuffer(int inBlockSize);

		unsigned char& operator[](int inIndex);
		bool PushDataToSource() override;
	};
}
