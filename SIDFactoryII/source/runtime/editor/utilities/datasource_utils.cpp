#include "runtime/editor/utilities/datasource_utils.h"
#include "runtime/editor/datasources/datasource_orderlist.h"
#include "runtime/editor/datasources/datasource_sequence.h"
#include <assert.h>

namespace Editor
{
	namespace DataSourceUtils
	{
		void ClearSequences(std::vector<std::shared_ptr<DataSourceSequence>>& inSequenceDataSources)
		{
			bool is_first = true;
			for (auto& sequence : inSequenceDataSources)
			{
				sequence->ClearEvents();
				sequence->SetLength(is_first ? 0x10 : 0x01);

				is_first = false;

				DataSourceSequence::PackResult packed_result = sequence->Pack();

				if (packed_result.m_DataLength < 0x100 && packed_result.m_Data != nullptr)
					sequence->SendPackedDataToBuffer(packed_result);

				sequence->PushDataToSource();
			}
		}


		void CopySequence(const std::shared_ptr<DataSourceSequence>& inSource, unsigned int inOffset, unsigned int inLength, const std::shared_ptr<DataSourceSequence>& inDestination)
		{
			assert(inSource != nullptr);
			assert(inDestination != nullptr);
			assert(inSource->GetLength() <= inOffset + inLength);
			assert(inDestination->GetLength() <= inLength);

			for (unsigned int i = 0; i < inLength; ++i)
				(*inDestination)[i] = (*inSource)[i + inOffset];
		}


		void ExpandSequences(std::vector<std::shared_ptr<DataSourceSequence>>& inSequenceDataSources)
		{
			for (auto& sequence : inSequenceDataSources)
			{
				ExpandSequence(sequence);
				DataSourceSequence::PackResult packed_result = sequence->Pack();

				if (packed_result.m_DataLength < 0x100 && packed_result.m_Data != nullptr)
					sequence->SendPackedDataToBuffer(packed_result);

				sequence->PushDataToSource();
			}
		}


		void ExpandSequence(const std::shared_ptr<DataSourceSequence>& inSequence)
		{
			DataSourceSequence& sequence = *inSequence;
			DataSourceSequence sequence_copy = DataSourceSequence(sequence);

			const int length = sequence_copy.GetLength();
			inSequence->SetLength(length * 2);

			int j = 0;
			for (int i = 0; i < length; ++i)
			{
				const DataSourceSequence::Event& event = sequence_copy[i];
				sequence[j + 0] = event;
				sequence[j + 1].Clear();
				sequence[j + 1].m_Note = event.m_Note != 0 ? 0x7e : 0;

				j += 2;
			}
		}


		void ShrinkSequence(const std::shared_ptr<DataSourceSequence>& inSequence)
		{
			DataSourceSequence& sequence = *inSequence;
			DataSourceSequence sequence_copy = DataSourceSequence(sequence);

			const int length = sequence_copy.GetLength();
			inSequence->SetLength(length / 2);

			int j = 0;
			for (int i = 0; i < length; i += 2)
			{
				const DataSourceSequence::Event& event = sequence_copy[i];
				sequence[j] = event;

				++j;
			}
		}


		void ClearOrderlist(std::vector<std::shared_ptr<DataSourceOrderList>>& inOrderListDataSources)
		{
			for (auto& order_list : inOrderListDataSources)
			{
				(*order_list)[0].m_Transposition = 0xa0;
				(*order_list)[0].m_SequenceIndex = 0;
				(*order_list)[1].m_Transposition = 0xff;
				(*order_list)[1].m_SequenceIndex = 0;

				for (unsigned int i = 2; i < order_list->GetLength(); ++i)
				{
					(*order_list)[i].m_Transposition = 0xa0;
					(*order_list)[i].m_SequenceIndex = 0;
				}

				order_list->ComputeLength();

				DataSourceOrderList::PackResult packed_result = order_list->Pack();

				if (packed_result.m_DataLength < 0x100)
					order_list->SendPackedDataToBuffer(packed_result);

				order_list->PushDataToSource();
			}
		}
	}
}