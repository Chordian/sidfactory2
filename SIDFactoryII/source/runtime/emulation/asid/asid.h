#pragma once

#include "runtime/editor/driver/driver_utils.h"
#include <vector>

#define ASID_NUM_REGS 28

class RtMidiOut;

namespace Emulation
{
	class ASid
	{
		static_assert(ASID_NUM_REGS < 32, "ASID_NUM_REGS must be less than 32");

	public:
		ASid(RtMidiOut* inRtMidiOut);

		void SendSIDRegisterWriteOrderAndCycleInfo(std::vector<Editor::SIDWriteInformation> inSIDWriteInfoList);
		void WriteToSIDRegister(unsigned char inSidReg, unsigned char inData);
		void SendToDevice();

	private:
		unsigned char GetASIDPositionFromRegisterIndex(unsigned char inSidRegister);
		
		RtMidiOut* m_RtMidiOut = nullptr;
		
		unsigned char ASIDRegisterBuffer[ASID_NUM_REGS];
		bool ASIDRegisterUpdated[ASID_NUM_REGS];
	};
}
