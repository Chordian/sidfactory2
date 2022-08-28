#include "runtime/editor/components/utils/orderlist_utils.h"
#include "runtime/editor/datasources/datasource_orderlist.h"


namespace Editor
{
	bool InsertSequenceIndexInOrderListAtIndex(std::shared_ptr<Editor::DataSourceOrderList>& inDataSourceOrderList, int inAtOrderListIndex, const DataSourceOrderList::Entry& inEntryToInsert)
	{
		if (inDataSourceOrderList->CanIncreaseSize())
		{
			inDataSourceOrderList->IncreaseSize();

			for (int i = inDataSourceOrderList->GetLength() - 2; i >= inAtOrderListIndex; --i)
				(*inDataSourceOrderList)[i + 1] = (*inDataSourceOrderList)[i];

			(*inDataSourceOrderList)[inAtOrderListIndex] = inEntryToInsert;

			return true;
		}

		return false;
	}


	void HandleOrderListUpdateAfterSequenceSplit(std::shared_ptr<Editor::DataSourceOrderList>& inDataSourceOrderList, unsigned char inSequenceIndex, unsigned char inAddSequenceIndex)
	{
		int length = inDataSourceOrderList->GetLength();

		for (int i = length - 1; i >= 0; --i)
		{
			const auto& entry = (*inDataSourceOrderList)[i];
			if (entry.m_Transposition <= 0xfe && entry.m_SequenceIndex == inSequenceIndex)
			{
				auto new_entry = entry;
				new_entry.m_SequenceIndex = inAddSequenceIndex;

				if (InsertSequenceIndexInOrderListAtIndex(inDataSourceOrderList, i + 1, new_entry))
					++length;
			}
		}
	}
}