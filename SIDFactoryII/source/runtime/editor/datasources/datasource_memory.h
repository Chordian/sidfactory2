#pragma once

#include "idatasource.h"

namespace Editor
{
	class DataSourceMemory : public IDataSource
	{
	protected:
		DataSourceMemory();
		DataSourceMemory(int inBlockSize);

	public:
		virtual ~DataSourceMemory();

		const int GetSize() const override;

	protected:
		int m_DataSize;
		unsigned char* m_Data;
	};
}
