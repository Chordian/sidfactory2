#include "cpuframecapture.h"
#include "cpumos6510.h"

namespace Emulation
{
	CPUFrameCapture::CPUFrameCapture(CPUmos6510* pCPU, unsigned short usCaptureRangeBegin, unsigned short usCaptureRangeEnd)
		: m_CPU(pCPU)
		, m_usCaptureRangeBegin(usCaptureRangeBegin)
		, m_usCaptureRangeEnd(usCaptureRangeEnd)
		, m_uiCurrentRead(0)
		, m_uiCyclesSpend(0)
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
		// Set program counter
		m_CPU->SetPC(inStartAddress);
		m_CPU->SetAccumulator(inAccumulatorValue);

		// Set the CPU to be not suspended
		m_CPU->SetSuspended(false);

		// Execute instructions until suspending!
		while (!m_CPU->IsSuspended())
			m_CPU->ExecuteInstruction();

		// Record the number of cycles spend on the executing code before the CPU was suspended!
		m_uiCyclesSpend = (unsigned int)m_CPU->CycleCounterGetCurrent();
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
