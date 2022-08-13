#pragma once

#include "runtime/editor/datasources/datasource_orderlist.h"

#include <memory>

namespace Editor
{
	bool InsertSequenceIndexInOrderListAtIndex(std::shared_ptr<Editor::DataSourceOrderList>& inOrderList, int inAtOrderListIndex, const DataSourceOrderList::Entry& inEntryToInsert);
}