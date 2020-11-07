#include "driver_architecture_sidfactory2.h"

#include "runtime/emulation/cpumemory.h"
#include "runtime/editor/driver/driver_info.h"

#include "foundation/base/assert.h"

namespace Editor
{
	void DriverArchitectureSidFactory2::SetEmptySequence(Emulation::CPUMemory* inCPUMemory, unsigned short inAddress)
	{
		FOUNDATION_ASSERT(inCPUMemory != nullptr);
		FOUNDATION_ASSERT(inCPUMemory->IsLocked());

		unsigned char empty_sequence_data[] = { 0x80, 0x00, 0x7f };
		inCPUMemory->SetData(inAddress, empty_sequence_data, sizeof(empty_sequence_data));
	}


	void DriverArchitectureSidFactory2::SetEmptyOrderList(Emulation::CPUMemory* inCPUMemory, unsigned short inAddress)
	{
		FOUNDATION_ASSERT(inCPUMemory != nullptr);
		FOUNDATION_ASSERT(inCPUMemory->IsLocked());

		unsigned char empty_orderlist_data[] = { 0xa0, 0x00, 0xff };
		inCPUMemory->SetData(inAddress, empty_orderlist_data, sizeof(empty_orderlist_data));
	}

	void DriverArchitectureSidFactory2::PostInitSetPlaybackIndices(std::vector<PlayMarkerInfo>& inPlayMarkerInfoList, Emulation::CPUMemory* inCPUMemory, const DriverInfo& inDriverInfo)
	{
		const auto& driver_common = inDriverInfo.GetDriverCommon();
		const auto& music_data = inDriverInfo.GetMusicData();

		const unsigned char track_count = music_data.m_TrackCount;

		if (track_count == inPlayMarkerInfoList.size())
		{
			const unsigned short order_list_index_address = driver_common.m_OrderListIndexAddress;
			const unsigned short sequence_index_address = driver_common.m_SequenceIndexAddress;
			const unsigned short tick_counter_address = driver_common.m_TickCounterAddress;
			const unsigned short current_sequence_address = driver_common.m_CurrentSequenceAddress;
			const unsigned short current_transpose_address = driver_common.m_CurrentTransposeAddress;
			const unsigned short current_event_duration_address = driver_common.m_CurrentSequenceEventDurationAddress;
			const unsigned short next_instrument_address = driver_common.m_NextInstrumentAddress;

			for (int i = 0; i < music_data.m_TrackCount; ++i)
			{
				(*inCPUMemory)[order_list_index_address + i] = inPlayMarkerInfoList[i].m_OrderListIndex;
				(*inCPUMemory)[sequence_index_address + i] = inPlayMarkerInfoList[i].m_SequenceIndex;
				(*inCPUMemory)[tick_counter_address + i] = inPlayMarkerInfoList[i].m_TickCounters;
				(*inCPUMemory)[current_sequence_address + i] = inPlayMarkerInfoList[i].m_CurrentSequence;
				(*inCPUMemory)[current_transpose_address + i] = inPlayMarkerInfoList[i].m_CurrentTranspose;
				(*inCPUMemory)[current_event_duration_address + i] = inPlayMarkerInfoList[i].m_CurrentSequenceEventDuration;
				(*inCPUMemory)[driver_common.m_SequenceInUseAddress + i] = inPlayMarkerInfoList[i].m_SequenceInUse ? 1 : 0;

				if (inPlayMarkerInfoList[i].m_NextInstrumentAddress >= 0x80)
					(*inCPUMemory)[next_instrument_address + i] = inPlayMarkerInfoList[i].m_NextInstrumentAddress;
			}
		}
	}
}