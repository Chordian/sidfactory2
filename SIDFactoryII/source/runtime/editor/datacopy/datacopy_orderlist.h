#pragma once

#include "runtime/editor/datasources/datasource_orderlist.h"
#include <vector>

namespace Editor
{
	class DataCopyOrderList
	{
	public:
		DataCopyOrderList();
		DataCopyOrderList(const std::vector<DataSourceOrderList::Entry>& inData);

		unsigned int GetEntryCount() const;
		const DataSourceOrderList::Entry& operator[](unsigned int inIndex) const;

	private:
		std::vector<DataSourceOrderList::Entry> m_Data;
	};
}