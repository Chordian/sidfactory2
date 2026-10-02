#include "cpuframecapture.h"
#include "cpumos6510.h"

namespace Emulation
{
	CPUFrameCapture::CPUFrameCapture(CPUmos6510* pCPU, unsigned short usCaptureRangeBegin, unsigned short usCaptureRangeEnd, unsigned int inMaxCycles)
		: m_CPU(pCPU)
		, m_usCaptureRangeBegin(usCaptureRangeBegin)
		, m_usCaptureRangeEnd(usCaptureRangeEnd)
		, m_uiMaxCycles(inMaxCycles)
		, m_uiCurrentRead(0)
		, m_uiCyclesSpend(0)
		, m_ReachedMaxCycleCount(false)
	{
		// Reset the CPU
		m_CPU->Reset();


		// Apply this class as write callback
		m_CPU->SetWriteCallback(this);
	}

	CPUFrameCapture::~CPUFrameCapture()
	{
		// Remove callback
		m_CPU->SetWriteCallback(nullptr);
	}

	//------------------------------------------------------------------------------------------------------

	void CPUFrameCapture::Capture(unsigned short inStartAddress, unsigned char inAccumulatorValue)
	{
		Capture(inStartAddress, inAccumulatorValue, 1);
	}

	void CPUFrameCapture::Capture(unsigned short inStartAddress, unsigned char inAccumulatorValue, int inEventlySpreadRepetitions)
	{
		FOUNDATION_ASSERT(inEventlySpreadRepetitions > 0);

		const int cyclesPerRepetition = m_uiMaxCycles / inEventlySpreadRepetitions;

		// Execute instructions until suspending!
		for(int i=0; i< inEventlySpreadRepetitions; ++i)
		{
			// Set program counter
			m_CPU->SetPC(inStartAddress);
			m_CPU->SetAccumulator(inAccumulatorValue);

			// Set the CPU to be not suspended
			m_CPU->SetSuspended(false);

			if(i > 0)
			{
				const int startCycle = cyclesPerRepetition * i;
				FOUNDATION_ASSERT(m_CPU->CycleCounterGetCurrent() <= startCycle);

				m_CPU->CycleCounterSetCurrent(startCycle);
			}

			while (!m_CPU->IsSuspended() && static_cast<unsigned int>(m_CPU->CycleCounterGetCurrent()) < m_uiMaxCycles)
				m_CPU->ExecuteInstruction();
		}

		// Record the number of cycles spend on the executing code before the CPU was suspended!
		m_uiCyclesSpend = static_cast<unsigned int>(m_CPU->CycleCounterGetCurrent());

		// Error state
		m_ReachedMaxCycleCount = m_uiCyclesSpend >= m_uiMaxCycles;
	}


	void CPUFrameCapture::Write(unsigned short usAddress, unsigned char ucVal, int iCycle)
	{
		if (usAddress >= m_usCaptureRangeBegin && usAddress <= m_usCaptureRangeEnd)
			m_aWrites.push_back(WriteCapture(usAddress, ucVal, iCycle));
	}

	const CPUFrameCapture::WriteCapture& CPUFrameCapture::GetNext()
	{
		m_uiCurrentRead++;
		return m_aWrites[m_uiCurrentRead - 1];
	}
}
