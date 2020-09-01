#pragma once

#include <memory>
#include <vector>

namespace Editor
{
	class DataSourceOrderList;
	class DataSourceSequence;

	namespace ComponentTrackUtils
	{
		struct FocusRow
		{
			int m_Row;
			int m_RowsAbove;
			int m_RowsBelow;
		};

		FocusRow CalculateFocusRow(int inFocusRow, int inHeight);

		unsigned int GetMaxEventPosition(const std::shared_ptr<DataSourceOrderList>& inOrderList, const std::vector<std::shared_ptr<DataSourceSequence>>& inSequenceList);
		unsigned int GetEventPosOf(unsigned int inOrderListIndex, unsigned int inSequencePosition, const std::shared_ptr<DataSourceOrderList>& inOrderList, const std::vector<std::shared_ptr<DataSourceSequence>>& inSequenceList);
	}
}