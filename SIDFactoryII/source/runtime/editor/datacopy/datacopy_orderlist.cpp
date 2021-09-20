#include "datacopy_orderlist.h"
#include "foundation/base/assert.h"

namespace Editor
{
	DataCopyOrderList::DataCopyOrderList()
	{
	}


	DataCopyOrderList::DataCopyOrderList(const std::vector<DataSourceOrderList::Entry>& inData)
		: m_Data(inData)
	{
	}


	unsigned int DataCopyOrderList::GetEntryCount() const
	{
		return static_cast<int>(m_Data.size());
	}


	const DataSourceOrderList::Entry& DataCopyOrderList::operator[](unsigned int inIndex) const
	{
		FOUNDATION_ASSERT(inIndex >= 0);
		FOUNDATION_ASSERT(inIndex < GetEntryCount());

		return m_Data[inIndex];
	}
}
