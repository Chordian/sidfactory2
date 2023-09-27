#pragma once

#include <memory>

namespace Foundation
{
	class IPlatform;
	class IMutex;
}

namespace Emulation
{
	class CPUMemory;

	class FlightRecorder
	{
	public:
		struct Frame
		{
			unsigned int m_nFrameNumber = 0;
			unsigned int m_nCyclesSpend = 0;
			unsigned char m_TempoCounter = 0;
			unsigned char m_DriverSync[3];
			unsigned char m_SIDData[0x19];

			void Reset()
			{
				m_nFrameNumber = 0;
				m_nCyclesSpend = 0;
				m_TempoCounter = 0;

				for (int i = 0; i < 3; ++i)
					m_DriverSync[i] = 0;
				for (int i = 0; i < 0x19; ++i)
					m_SIDData[i] = 0;
			}
		};

		FlightRecorder(Foundation::IPlatform* inPlatform, unsigned int inFrameCapacity);
		~FlightRecorder();

		void Lock();
		void Unlock();

		void SetDriverSyncReadAddress(unsigned short inDriverSyncReadAddress);
		void SetDriverTempoCounterReadAddress(unsigned short inDriverTempoCounterReadAddress);

		void SetRecording(bool inRecord);
		bool IsRecording() const;

		void Reset();
		void Record(unsigned int inFrame, CPUMemory* inMemory, unsigned int inCyclesSpend);

		unsigned int RecordedFrameCount() const;
		const Frame& GetFrame(unsigned int inIndex) const;
		const Frame& GetNewestFrame() const;

		const unsigned int GetCapacity() const { return m_FrameCapacity; }

	private:
		void RecordFrame(unsigned int inFrame, CPUMemory* inMemory, unsigned int inCyclesSpend, Frame& inFrameData);

		std::shared_ptr<Foundation::IMutex> m_Mutex;

		bool m_IsRecording;

		unsigned short m_DriverSyncAddress;
		unsigned short m_DriverTempoCounterAddress;

		unsigned int m_TopIndex;
		unsigned int m_RecordedFrameCount;

		unsigned int m_FrameCapacity;

		bool m_Locked;

		Frame m_LastRecordedFrame;
		Frame* m_Frames;
	};
}
