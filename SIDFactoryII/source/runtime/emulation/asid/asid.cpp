#include "asid.h"

#include "libraries/rtmidi/RtMidi.h"

#include "utils/configfile.h"
#include "utils/global.h"
#include "utils/logging.h"

#include <array>
#include <vector>

namespace Emulation
{
	ASid::ASid(RtMidiOut* inRtMidiOut)
		: m_RtMidiOut(inRtMidiOut)
	{
		using namespace Utility;
		
		// Reset the ASID buffer
		for (unsigned int i = 0; i < ASID_NUM_REGS; ++i)
		{
			ASIDRegisterBuffer[i] = 0;
			ASIDRegisterUpdated[i] = false;
		}
	}

	void ASid::SendSIDRegisterWriteOrderAndCycleInfo(std::vector<Editor::SIDWriteInformation> inSIDWriteInfoList)
	{
		#define WRITE_ORDER_UNUSED 0xff
		
		struct write_order
		{
			unsigned char ucIndex;
			unsigned char ucCycles;
		};
		
		// Physical out buffer, including protocol overhead
		unsigned char aucAsidOutBuffer[ASID_NUM_REGS*2+4];

		// Write order list, initialized with all slots unused
		std::array<write_order, ASID_NUM_REGS> aWriteOrder;
		aWriteOrder.fill({0, WRITE_ORDER_UNUSED});

		// Get the write order from the analysis, for all the three voices
		int index = 0;
		int iPrevASIDreg = -1;
		
		for (int voice = 0; voice < 3 ; ++voice)
		{
			unsigned char ucLastCycleOffset = 0;
			for (const auto& SIDWriteInfo : inSIDWriteInfoList)
			{
				unsigned char ucASIDreg;
				ucASIDreg = GetASIDPositionFromRegisterIndex((SIDWriteInfo.m_AddressLow & 0xff) + 7 * voice);
				aWriteOrder[ucASIDreg].ucIndex = index;

				// The cycles are specified as "post wait times", so will be used on the next cycle value
				if (iPrevASIDreg >= 0)
				{
					aWriteOrder[iPrevASIDreg].ucCycles = SIDWriteInfo.m_CycleOffset-ucLastCycleOffset;
					ucLastCycleOffset = SIDWriteInfo.m_CycleOffset;
				}
				
				iPrevASIDreg = ucASIDreg;
				++index;
			}
		}
		
		// Last register does not need to wait any cycles
		aWriteOrder[iPrevASIDreg].ucCycles = 0;

		// Add the remaining registers last, without waits
		for (auto& wo : aWriteOrder)
		{
			if (wo.ucCycles == WRITE_ORDER_UNUSED)
			{
				wo.ucIndex = index++;
				wo.ucCycles = 0;
			}
		}

		// Sysex start data for an ASID message
		index = 0;
		
		aucAsidOutBuffer[index++] = 0xf0;
		aucAsidOutBuffer[index++] = 0x2d;
		aucAsidOutBuffer[index++] = 0x30; // Write order

		// Fill in the write order payload. Only 7 bits may be used in MIDI
		for (auto& wo : aWriteOrder)
		{
			// Index serves double duty as index (5 bits) and msb (1 bit) of cycles
			aucAsidOutBuffer[index++] = (wo.ucIndex & 0x1f) + ((wo.ucCycles & 0x80) >> 1);
			aucAsidOutBuffer[index++] = wo.ucCycles & 0x7f;
		}

		// Sysex end marker
		aucAsidOutBuffer[index++] = 0xf7;
		
		// Send to physical MIDI port
		if (m_RtMidiOut->isPortOpen())
			m_RtMidiOut->sendMessage(aucAsidOutBuffer, index);
	}
	
	void ASid::WriteToSIDRegister(unsigned char inSidReg, unsigned char inData)
	{
		if (inSidReg > 0x18) 
			return;

		// Get the ASID transformed register
		unsigned char ucMappedAddr = GetASIDPositionFromRegisterIndex(inSidReg);

		// If a write occurs to a waveform register, check if first block is already allocated
		if (ucMappedAddr >= 0x16 && ucMappedAddr <= 0x18 && ASIDRegisterUpdated[ucMappedAddr])
		{
			ucMappedAddr += 3;

			// If second block is also updated, move it to the first to make sure to always keep the last one
			if(ASIDRegisterUpdated[ucMappedAddr])
				ASIDRegisterBuffer[ucMappedAddr - 3] = ASIDRegisterBuffer[ucMappedAddr];
		}

		// If we're trying to update a control register that is already mapped, flush it directly
		if(ASIDRegisterUpdated[ucMappedAddr])
		{
			if(ucMappedAddr >= 0x16)
				SendToDevice();
		}

		// Store the data
		ASIDRegisterBuffer[ucMappedAddr] = inData;
		ASIDRegisterUpdated[ucMappedAddr] = true;
	}
	
	void ASid::SendToDevice()
	{
		// Physical out buffer, including protocol overhead
		static unsigned char aucAsidOutBuffer[ASID_NUM_REGS+12];

		const bool RequireUpdate = [&RegisterUpdated = ASIDRegisterUpdated]()
		{
			for (unsigned int i = 0; i < ASID_NUM_REGS; ++i)
			{
				if (RegisterUpdated[i])
					return true;
			}

			return false;
		}();
		
		if (!RequireUpdate)
			return;

		if (m_RtMidiOut->isPortOpen())
		{
			// Sysex start data for an ASID message
			aucAsidOutBuffer[0] = 0xf0;
			aucAsidOutBuffer[1] = 0x2d;
			aucAsidOutBuffer[2] = 0x4e;
		
			size_t index = 3;

			// Setup mask bytes (one bit per register)
			unsigned char ucReg;
		
			for (unsigned char ucMask = 0; ucMask<4; ++ucMask)
			{
				ucReg = 0x00;
				for (unsigned char ucRegOffset = 0; ucRegOffset < 7; ++ucRegOffset)
				{
					if (ASIDRegisterUpdated[ucMask*7+ucRegOffset])
						ucReg |= (1<<ucRegOffset);
				}
				aucAsidOutBuffer[index++] = ucReg;
			}

			// Setup the MSB bits, one per register (since MIDI only allows for 7-bit data bytes)
			for (unsigned char ucMsb=0; ucMsb<4; ++ucMsb)
			{
				ucReg = 0x00;
				for (unsigned char ucRegOffset = 0; ucRegOffset < 7; ++ucRegOffset)
				{
					if (ASIDRegisterBuffer[ucMsb*7 + ucRegOffset] & 0x80)
						ucReg |= (1 << ucRegOffset);
				}
				aucAsidOutBuffer[index++] = ucReg;
			}

			// Add data for all updated registers (the 7 LSB bits)
			for (unsigned int i = 0; i < ASID_NUM_REGS; ++i)
			{
				if (ASIDRegisterUpdated[i])
					aucAsidOutBuffer[index++] = ASIDRegisterBuffer[i] & 0x7f;
			}

			// Sysex end marker
			aucAsidOutBuffer[index++] = 0xf7;

			// Send to physical MIDI port
			m_RtMidiOut->sendMessage(aucAsidOutBuffer, index);
		}

		// Prepare for next buffer
		for (int i = 0; i < ASID_NUM_REGS; ++i)
			ASIDRegisterUpdated[i] = false;
	}

	unsigned char ASid::GetASIDPositionFromRegisterIndex(unsigned char inSidRegister)
	{
		// Conversion between SID register and ASID position
		static const unsigned char SIDRegisterMap[] =
		{
			0x00, 0x01, 0x02, 0x03, 0x16, 0x04, 0x05,
			0x06, 0x07, 0x08, 0x09, 0x17, 0x0a, 0x0b,
			0x0c, 0x0d, 0x0e, 0x0f, 0x18, 0x10, 0x11,
			0x12, 0x13, 0x14, 0x15, 0x19, 0x1a, 0x1b
		};

		return SIDRegisterMap[inSidRegister];
	}
}
