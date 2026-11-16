#pragma once

#include "runtime/editor/driver/driver_utils.h"
#include <vector>

#define ASID_NUM_REGS 28

// SID 1 uses command 0x4e, SID 2 to 4 use 0x50 to 0x52
#define ASID_MAX_SIDS 4

class RtMidiOut;

namespace Emulation
{
	class ASid
	{
	public:
		ASid(RtMidiOut* inRtMidiOut);

		bool isPortOpen();
		void SetMuted(bool inMuted);
		void SetSIDCount(unsigned int inSIDCount);

		void SendSIDRegisterWriteOrderAndCycleInfo(std::vector<Editor::SIDWriteInformation> inSIDWriteInfoList);
		void SendSIDType(bool is6581);
		void SendSIDEnvironment(bool isPAL);
		void WriteToSIDRegister(unsigned int inSIDIndex, unsigned char inSidReg, unsigned char inData);
		void SendToDevice();

	private:
		void SendSetChannelsSilent();
		void SendSIDRegisters(unsigned int inSIDIndex);
		unsigned char GetASIDPositionFromRegisterIndex(unsigned char inSidRegister);

		bool m_Muted = false;
		unsigned int m_SIDCount = 1;
		RtMidiOut* m_RtMidiOut = nullptr;

		// Physical out buffer, including protocol overhead
		unsigned char m_ASIDOutBuffer[ASID_NUM_REGS + 12];

		// Registers, one set per SID
		unsigned char m_ASIDRegisterBuffer[ASID_MAX_SIDS][ASID_NUM_REGS];
		bool m_ASIDRegisterUpdated[ASID_MAX_SIDS][ASID_NUM_REGS];
	};
}
