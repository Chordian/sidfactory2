#pragma once

#include "idatasource.h"
#include <vector>
#include <assert.h>

namespace Editor
{
	template<typename LIST_ITEM_TYPE>
	class DataSourceTList : public IDataSource
	{
	public:
		DataSourceTList();
		DataSourceTList(std::vector<LIST_ITEM_TYPE>&& inItems);

		virtual LIST_ITEM_TYPE& operator[](int inIndex);
		virtual bool PushDataToSource() override;

		const int GetSize() const override;

	protected:
		std::vector<LIST_ITEM_TYPE> m_List;
	};

	template<typename LIST_ITEM_TYPE>
	DataSourceTList<LIST_ITEM_TYPE>::DataSourceTList()
	{
	}

	template<typename LIST_ITEM_TYPE>
	DataSourceTList<LIST_ITEM_TYPE>::DataSourceTList(std::vector<LIST_ITEM_TYPE>&& inItems)
	{
		m_List = inItems;
	}


	template<typename LIST_ITEM_TYPE>
	LIST_ITEM_TYPE& DataSourceTList<LIST_ITEM_TYPE>::operator[](int inIndex)
	{
		assert(inIndex >= 0);
		assert(inIndex < static_cast<int>(m_List.size()));

		return m_List[inIndex];
	}

	template<typename LIST_ITEM_TYPE>
	bool DataSourceTList<LIST_ITEM_TYPE>::PushDataToSource()
	{
		return true;
	}


	template<typename LIST_ITEM_TYPE>
	const int DataSourceTList<LIST_ITEM_TYPE>::GetSize() const
	{
		return static_cast<int>(m_List.size());
	}
}
