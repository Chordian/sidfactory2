#include "runtime/editor/components/utils/orderlist_utils.h"


namespace Editor
{
	bool OrderListInsert(std::shared_ptr<Editor::DataSourceOrderList>& inOrderList, int inAtOrderListIndex, const DataSourceOrderList::Entry& inEntryToInsert)
	{
		if (inOrderList->CanIncreaseSize())
		{
			inOrderList->IncreaseSize();

			for (int i = inOrderList->GetLength() - 2; i >= inAtOrderListIndex; --i)
				(*inOrderList)[i + 1] = (*inOrderList)[i];

			(*inOrderList)[inAtOrderListIndex] = inEntryToInsert;

			return true;
		}

		return false;
	}
}