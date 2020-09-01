#if !defined(__CPUFRAME_H__)
#define __CPUFRAME_H__

#include "icpuwritecallback.h"
#include <vector>

namespace Emulation
{
	class CPUmos6510;
	class CPUFrameCapture : public ICPUWriteCallback
	{
	public:
		struct WriteCapture
		{
			unsigned short m_usReg;
			unsigned char m_ucVal;
			int m_iCycle;

			WriteCapture()
				: m_usReg(0)
				, m_ucVal(0)
				, m_iCycle(-1)
			{

			}

			WriteCapture(unsigned short usReg, unsigned char ucVal, int iCycle)
				: m_usReg(usReg)
				, m_ucVal(ucVal)
				, m_iCycle(iCycle)
			{

			}
		};

		// Note: Capture range begin and end values are included 
		CPUFrameCapture(CPUmos6510* pCPU, unsigned short usCaptureRangeBegin, unsigned short usCaptureRangeEnd);
		~CPUFrameCapture();

		void Capture(unsigned short inStartAddress, unsigned char inAccumulatorValue);

		virtual void Write(unsigned short usAddress, unsigned char ucVal, int iCycle);

		unsigned int GetCyclesSpend() const { return m_uiCyclesSpend; }

		const WriteCapture& GetNext();
		bool HasNext() const { return m_aWrites.size() > m_uiCurrentRead; }
		bool IsEmpty() const { return m_aWrites.size() == 0; }

	private:
		CPUmos6510* m_CPU;

		unsigned short m_usCaptureRangeBegin;
		unsigned short m_usCaptureRangeEnd;

		unsigned int m_uiCurrentRead;

		unsigned int m_uiCyclesSpend;

		std::vector<WriteCapture> m_aWrites;
	};
}

#endif //__CPUFRAME_H__
