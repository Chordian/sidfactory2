#include "flightrecorder.h"

#include "foundation/platform/iplatform.h"
#include "foundation/platform/imutex.h"
#include "runtime/emulation/cpumemory.h"
#include <memory>
#include "foundation/base/assert.h"

namespace Emulation
{
	FlightRecorder::FlightRecorder(Foundation::IPlatform* inPlatform, unsigned int inCapacity)
		: m_FrameCapacity(inCapacity)
		, m_Locked(false)
		, m_IsRecording(false)
		, m_TopIndex(0)
		, m_RecordedFrameCount(0)
		, m_DriverSyncAddress(0x0000)
	{
		m_Mutex = inPlatform->CreateMutex();

		m_Frames = new Frame[m_FrameCapacity];

		for (unsigned int i = 0; i < m_FrameCapacity; ++i)
			m_Frames[i].Reset();
	}

	FlightRecorder::~FlightRecorder()
	{
		delete[] m_Frames;
	}

	//------------------------------------------------------------------------------------------------

	void FlightRecorder::Lock()
	{
		m_Mutex->Lock();
		m_Locked = true;
	}

	void FlightRecorder::Unlock()
	{
		m_Locked = false;
		m_Mutex->Unlock();
	}

	//------------------------------------------------------------------------------------------------

	void FlightRecorder::SetDriverSyncReadAddress(unsigned short inDriverSyncReadAddress)
	{
		m_DriverSyncAddress = inDriverSyncReadAddress;
	}

	void FlightRecorder::SetDriverTempoCounterReadAddress(unsigned short inDriverTempoCounterReadAddress)
	{
		m_DriverTempoCounterAddress = inDriverTempoCounterReadAddress;
	}

	//------------------------------------------------------------------------------------------------

	void FlightRecorder::SetRecording(bool inRecording)
	{
		m_IsRecording = inRecording;
	}

	bool FlightRecorder::IsRecording() const
	{
		return m_IsRecording;
	}

	//------------------------------------------------------------------------------------------------

	void FlightRecorder::Reset()
	{
		FOUNDATION_ASSERT(m_Locked);

		m_TopIndex = 0;
		m_RecordedFrameCount = 0;
		m_CyclesSpendMax = 0;

		for (unsigned int i = 0; i < m_FrameCapacity; ++i)
			m_Frames[i].Reset();
	}

	//------------------------------------------------------------------------------------------------

	void FlightRecorder::Record(unsigned int inFrame, CPUMemory* inMemory, unsigned int inCyclesSpend)
	{
		FOUNDATION_ASSERT(m_Locked);
		FOUNDATION_ASSERT(m_IsRecording);

		if (inMemory != nullptr)
		{
			if (m_RecordedFrameCount < m_FrameCapacity)
			{
				FOUNDATION_ASSERT(m_TopIndex == 0);

				Frame& frame = m_Frames[m_RecordedFrameCount];
				RecordFrame(inFrame, inMemory, inCyclesSpend, frame);

				m_LastRecordedFrame = frame;

				m_RecordedFrameCount++;
			}
			else
			{
				FOUNDATION_ASSERT(m_RecordedFrameCount == m_FrameCapacity);
				FOUNDATION_ASSERT(m_TopIndex < m_FrameCapacity);

				Frame& frame = m_Frames[m_TopIndex];
				RecordFrame(inFrame, inMemory, inCyclesSpend, frame);

				m_LastRecordedFrame = frame;
				
				m_TopIndex++;

				if (m_TopIndex >= m_FrameCapacity)
					m_TopIndex = 0;
			}
		}
	}

	unsigned int FlightRecorder::RecordedFrameCount() const
	{
		FOUNDATION_ASSERT(m_Locked);
		return m_RecordedFrameCount;
	}

	unsigned int FlightRecorder::CyclesSpendMax() const
	{
		FOUNDATION_ASSERT(m_Locked);
		return m_CyclesSpendMax;
	}

	//------------------------------------------------------------------------------------------------

	const FlightRecorder::Frame& FlightRecorder::GetFrame(unsigned int inIndex) const
	{
		FOUNDATION_ASSERT(m_Locked);

		inIndex += m_TopIndex;

		if (inIndex >= m_FrameCapacity)
			inIndex -= m_FrameCapacity;

		FOUNDATION_ASSERT(inIndex >= 0 && inIndex < m_FrameCapacity);

		return m_Frames[inIndex];
	}

	const FlightRecorder::Frame& FlightRecorder::GetNewestFrame() const
	{
		return m_LastRecordedFrame;	
	}

	//------------------------------------------------------------------------------------------------

	void FlightRecorder::RecordFrame(unsigned int inFrame, CPUMemory* inMemory, unsigned int inCyclesSpend, Frame& inFrameData)
	{
		FOUNDATION_ASSERT(m_Locked);

		inFrameData.m_nFrameNumber = inFrame;
		inFrameData.m_nCyclesSpend = inCyclesSpend;

		if (inCyclesSpend > m_CyclesSpendMax) {
			m_CyclesSpendMax = inCyclesSpend;
		}

		inMemory->GetData(0xd400, &inFrameData.m_SIDData, 0x19);

		inFrameData.m_TempoCounter = (*inMemory)[m_DriverTempoCounterAddress];

		for (int i = 0; i < 3; ++i)
			inFrameData.m_DriverSync[i] = (*inMemory)[m_DriverSyncAddress + i];
	}
}
