#pragma once

#include <vector>

namespace Emulation
{
	class CPUMemory;
}

namespace Editor
{
	class DriverInfo;
	class IDriverArchitecture
	{
	public:
		struct PlayMarkerInfo
		{
			unsigned char m_OrderListIndex;
			unsigned char m_CurrentSequence;
			unsigned char m_CurrentTranspose;
			unsigned char m_SequenceIndex;
			unsigned char m_CurrentSequenceEventDuration;
			unsigned char m_TickCounters;
			unsigned char m_NextInstrumentAddress;
			bool m_SequenceInUse;
		};

		virtual ~IDriverArchitecture() { }

		virtual void SetEmptySequence(Emulation::CPUMemory* inCPUMemory, unsigned short inAddress) = 0;
		virtual void SetEmptyOrderList(Emulation::CPUMemory* inCPUMemory, unsigned short inAddress) = 0;
		virtual void PostInitSetPlaybackIndices(std::vector<PlayMarkerInfo>& inPlayMarkerInfoList, Emulation::CPUMemory* inCPUMemory, const DriverInfo& inDriverInfo) = 0;
	};
}