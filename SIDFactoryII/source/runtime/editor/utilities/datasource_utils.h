#pragma once

#include <vector>
#include <memory>

namespace Editor
{
	class DataSourceSequence;
	class DataSourceOrderList;

	namespace DataSourceUtils
	{
		// Sequence
		void ClearSequences(std::vector<std::shared_ptr<DataSourceSequence>>& inSequenceDataSources);
		void CopySequence(const std::shared_ptr<DataSourceSequence>& inSource, unsigned int inOffset, unsigned int inLength, const std::shared_ptr<DataSourceSequence>& inDestination);
		void ExpandSequences(std::vector<std::shared_ptr<DataSourceSequence>>& inSequenceDataSources);
		void ExpandSequence(const std::shared_ptr<DataSourceSequence>& inSequence);
		void ShrinkSequence(const std::shared_ptr<DataSourceSequence>& inSequence);
		
		// Orderlist
		void ClearOrderlist(std::vector<std::shared_ptr<DataSourceOrderList>>& inOrderListDataSources);
	}
}