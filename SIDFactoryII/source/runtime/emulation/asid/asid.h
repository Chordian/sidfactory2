#pragma once

#include "runtime/editor/driver/driver_utils.h"
#include <vector>

#define ASID_NUM_REGS 28

class RtMidiOut;

namespace Emulation
{
	class ASid
	{
	public:
		ASid(RtMidiOut* inRtMidiOut);

		void SetMuted(bool inMuted);
		
		void SendSIDRegisterWriteOrderAndCycleInfo(std::vector<Editor::SIDWriteInformation> inSIDWriteInfoList);
		void WriteToSIDRegister(unsigned char inSidReg, unsigned char inData);
		void SendToDevice();

	private:
		unsigned char GetASIDPositionFromRegisterIndex(unsigned char inSidRegister);

		bool m_Muted = false;
		RtMidiOut* m_RtMidiOut = nullptr;
		
		// Physical out buffer, including protocol overhead
		unsigned char m_ASIDOutBuffer[ASID_NUM_REGS + 12];

		// Registers
		unsigned char m_ASIDRegisterBuffer[ASID_NUM_REGS];
		bool m_ASIDRegisterUpdated[ASID_NUM_REGS];
	};
}
