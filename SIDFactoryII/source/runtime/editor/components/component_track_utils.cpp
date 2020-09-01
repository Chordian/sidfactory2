#include "component_track_utils.h"

#include "runtime/editor/datasources/datasource_orderlist.h"
#include "runtime/editor/datasources/datasource_sequence.h"

#include <assert.h>

namespace Editor
{
	namespace ComponentTrackUtils
	{
		FocusRow CalculateFocusRow(int inFocusRow, int inHeight)
		{
			return FocusRow(
				{
					inFocusRow,
					inFocusRow,
					inHeight - inFocusRow - 1
				}
			);
		}


		unsigned int GetMaxEventPosition(const std::shared_ptr<DataSourceOrderList>& inOrderList, const std::vector<std::shared_ptr<DataSourceSequence>>& inSequenceList)
		{
			assert(inOrderList != nullptr);
			assert(inSequenceList.size() > 0);

			unsigned int max_event_position = 0;

			for (unsigned int i = 0; i < inOrderList->GetLength(); ++i)
			{
				const unsigned char transposition = (*inOrderList)[i].m_Transposition;

				if (transposition < 0xfe)
				{
					const unsigned char sequence_index = (*inOrderList)[i].m_SequenceIndex;
					max_event_position += inSequenceList[sequence_index]->GetLength();
				}
			}

			return max_event_position;
		}


		unsigned int GetEventPosOf(unsigned int inOrderListIndex, unsigned int inSequencePosition, const std::shared_ptr<DataSourceOrderList>& inOrderList, const std::vector<std::shared_ptr<DataSourceSequence>>& inSequenceList)
		{
			assert(inOrderList != nullptr);
			assert(inOrderListIndex < inOrderList->GetLength());
			assert(inSequenceList.size() > 0);

			unsigned int max_event_position = 0;

			for (unsigned int i = 0; i < inOrderListIndex; ++i)
			{
				const unsigned char transposition = (*inOrderList)[i].m_Transposition;

				if (transposition < 0xfe)
				{
					const unsigned char sequence_index = (*inOrderList)[i].m_SequenceIndex;
					max_event_position += inSequenceList[sequence_index]->GetLength();
				}
			}

			return max_event_position + inSequencePosition;
		}
	}
}