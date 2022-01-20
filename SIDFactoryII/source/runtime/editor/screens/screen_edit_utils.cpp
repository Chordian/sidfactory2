#include "screen_edit_utils.h"
#include "runtime/editor/driver/driver_info.h"
#include "runtime/editor/driver/driver_state.h"
#include "runtime/editor/driver/driver_utils.h"
#include "runtime/editor/driver/idriver_architecture.h"
#include "runtime/editor/datasources/datasource_orderlist.h"
#include "runtime/editor/datasources/datasource_sequence.h"
#include "runtime/emulation/cpumemory.h"

#include "foundation/base/assert.h"

namespace Editor
{
	namespace ScreenEditUtils
	{
		void PrepareSequenceData(const Editor::DriverInfo& inDriverInfo, Emulation::CPUMemory& inCPUMemory)
		{
			FOUNDATION_ASSERT(inCPUMemory.IsLocked());

			if (inDriverInfo.IsValid())
			{
				// Find highest used sequence index
				const unsigned char highest_sequence_index = DriverUtils::GetHighestSequenceIndexUsed(inDriverInfo, inCPUMemory);

				// Get Music data descriptor
				const DriverInfo::MusicData& music_data = inDriverInfo.GetMusicData();

				// Reset contents of any subsequent sequences
				IDriverArchitecture* driver_architecture = inDriverInfo.GetDriverArchitecture();
				
				for (int i = highest_sequence_index + 1; i < music_data.m_SequenceCount; ++i)
				{
					const unsigned short sequence_address = music_data.m_Sequence00Address + i * music_data.m_SequenceSize;
					driver_architecture->SetEmptySequence(&inCPUMemory, sequence_address);
				}
			}
		}


		void PrepareSequencePointers(const Editor::DriverInfo& inDriverInfo, Emulation::CPUMemory& inCPUMemory)
		{
			FOUNDATION_ASSERT(inCPUMemory.IsLocked());

			if (inDriverInfo.IsValid())
			{
				// Get Music data descriptor
				const DriverInfo::MusicData& music_data = inDriverInfo.GetMusicData();

				for (int i = 0; i < music_data.m_SequenceCount; ++i)
				{
					const unsigned short sequence_address = music_data.m_Sequence00Address + i * music_data.m_SequenceSize;
					
					const unsigned char sequence_address_low = static_cast<unsigned char>(sequence_address & 0xff);
					const unsigned char sequence_address_high = static_cast<unsigned char>(sequence_address >> 8);

					inCPUMemory[music_data.m_SequencePointersLowAddress + i] = sequence_address_low;
					inCPUMemory[music_data.m_SequencePointersHighAddress + i] = sequence_address_high;
				}
			}
		}


		void PrepareOrderListsDataSources(const Editor::DriverInfo& inDriverInfo, Emulation::CPUMemory& inCPUMemory, std::vector<std::shared_ptr<DataSourceOrderList>>& outOrderListDataSources)
		{
			const DriverInfo::MusicData& music_data = inDriverInfo.GetMusicData();
			const unsigned short order_list_data_size = music_data.m_OrderListSize;

			for (unsigned int i = 0; i < music_data.m_TrackCount; ++i)
			{
				const unsigned short order_list_data_address = music_data.m_OrderListTrack1Address + i * order_list_data_size;
				outOrderListDataSources.push_back(std::make_shared<DataSourceOrderList>(
					&inCPUMemory, 
					order_list_data_address, 
					order_list_data_size));
			}
		}


		void PrepareSequenceDataSources(const Editor::DriverInfo& inDriverInfo, const Editor::DriverState& inDriverState, Emulation::CPUMemory& inCPUMemory, std::vector<std::shared_ptr<DataSourceSequence>>& outSequenceDataSources)
		{
			const DriverInfo::MusicData& music_data = inDriverInfo.GetMusicData();
			const unsigned short sequence_data_size = music_data.m_SequenceSize;

			for (int i = 0; i < music_data.m_SequenceCount; ++i)
			{
				const unsigned short sequence_data_address = music_data.m_Sequence00Address + i * sequence_data_size;
				outSequenceDataSources.push_back(std::make_shared<DataSourceSequence>(
					&inCPUMemory, 
					inDriverInfo,
					inDriverState,
					static_cast<unsigned char>(i), 
					sequence_data_address, 
					sequence_data_size
					)
				);
			}
		}
	}
}