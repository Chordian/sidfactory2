#include "asid.h"

#include "libraries/rtmidi/RtMidi.h"
#include "runtime/environmentdefines.h"

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
			m_ASIDRegisterBuffer[i] = 0;
			m_ASIDRegisterUpdated[i] = false;
		}
	}


	bool ASid::isPortOpen() 
	{
		return m_RtMidiOut->isPortOpen();
	}

	void ASid::SetMuted(bool inMuted)
	{
		if(inMuted == m_Muted)
			return;

		if(inMuted)
			SendSetChannelsSilent();

		m_Muted = inMuted;
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
		unsigned char ASidOutBuffer[ASID_NUM_REGS*2+4];

		// Write order list, initialized with all slots unused
		std::array<write_order, ASID_NUM_REGS> WriteOrder;
		WriteOrder.fill({0, WRITE_ORDER_UNUSED});

		// Get the write order from the analysis, for all the three voices
		int index = 0;
		int PreviousSidRegister = -1;
		
		for (int voice = 0; voice < 3 ; ++voice)
		{
			unsigned char PreviousCycleOffset = 0;
			for (const auto& SIDWriteInfo : inSIDWriteInfoList)
			{
				unsigned char ucASIDreg;
				ucASIDreg = GetASIDPositionFromRegisterIndex((SIDWriteInfo.m_AddressLow & 0xff) + 7 * voice);
				WriteOrder[ucASIDreg].ucIndex = index;

				// The cycles are specified as "post wait times", so will be used on the next cycle value
				if (PreviousSidRegister >= 0)
				{
					WriteOrder[PreviousSidRegister].ucCycles = SIDWriteInfo.m_CycleOffset - PreviousCycleOffset;
					PreviousCycleOffset = SIDWriteInfo.m_CycleOffset;
				}
				
				PreviousSidRegister = ucASIDreg;
				++index;
			}
		}
		
		// Last register does not need to wait any cycles
		WriteOrder[PreviousSidRegister].ucCycles = 0;

		// Add the remaining registers last, without waits
		for (auto& wo : WriteOrder)
		{
			if (wo.ucCycles == WRITE_ORDER_UNUSED)
			{
				wo.ucIndex = index++;
				wo.ucCycles = 0;
			}
		}

		// Sysex start data for an ASID message
		index = 0;
		
		ASidOutBuffer[index++] = 0xf0;
		ASidOutBuffer[index++] = 0x2d;
		ASidOutBuffer[index++] = 0x30; // Write order

		// Fill in the write order payload. Only 7 bits may be used in MIDI
		for (auto& wo : WriteOrder)
		{
			// Index serves double duty as index (5 bits) and msb (1 bit) of cycles
			ASidOutBuffer[index++] = (wo.ucIndex & 0x1f) + ((wo.ucCycles & 0x80) >> 1);
			ASidOutBuffer[index++] = wo.ucCycles & 0x7f;
		}

		// Sysex end marker
		ASidOutBuffer[index++] = 0xf7;
		
		// Send to physical MIDI port
		if (m_RtMidiOut->isPortOpen())
			m_RtMidiOut->sendMessage(ASidOutBuffer, index);
	}

	void ASid::SendSIDEnvironment(bool isPAL)
	{
		// Physical out buffer, including protocol overhead
		unsigned char ASidOutBuffer[8];
		int index = 0;

		// No UI exist to request buffering or change speed multiplier
		const bool isBufferingRequested = false;
		const int speedMultiplier = 1; // 1x

		// Time between two frames
		const int frameDeltaUs = isPAL ? (long)1000000*63*312/EMULATION_CYCLES_PER_SECOND_PAL : (long)1000000*65*263/EMULATION_CYCLES_PER_SECOND_NTSC;

		// Sysex start data for an ASID message
		ASidOutBuffer[index++] = 0xf0;
		ASidOutBuffer[index++] = 0x2d;
		ASidOutBuffer[index++] = 0x31; // SID environment

		// Payload

		/* data0: settings
			bit0    : 0 = PAL, 1 = NTSC
			bits4-1 : speed, 1x to 16x (value 0-15)
			bit5    : 1 = custom speed (only framedelta valid)
			bit6    : 1 = buffering requested by user
		 */
		ASidOutBuffer[index++] =
			((isBufferingRequested ? 1:0)   << 6) |
			(((speedMultiplier - 1) & 0x0f) << 1) |
			((isPAL? 0:1)                   << 0);

		/* data1: framedelta uS, total 7+7+2=16 bits, slowest time = 65535us = 15Hz
			bits6-0: framedelta uS (LSB)

		   data2:
			bits6-0: framedelta uS

		   data3:
			bits1-0: framedelta uS (MSB)
			bits6-2: 5 bits (reserved)
		*/
		ASidOutBuffer[index++] = (frameDeltaUs >> 7*0) & 0x7f;
		ASidOutBuffer[index++] = (frameDeltaUs >> 7*1) & 0x7f;
		ASidOutBuffer[index++] = (frameDeltaUs >> 7*2) & 0x03;

		// Sysex end marker
		ASidOutBuffer[index++] = 0xf7;

		// Send to physical MIDI port
		if (m_RtMidiOut->isPortOpen())
			m_RtMidiOut->sendMessage(ASidOutBuffer, index);
	}

	void ASid::SendSIDType(bool is6581)
	{
		// Physical out buffer, including protocol overhead
		unsigned char ASidOutBuffer[6];
		int index = 0;

		// Sysex start data for an ASID message
		ASidOutBuffer[index++] = 0xf0;
		ASidOutBuffer[index++] = 0x2d;
		ASidOutBuffer[index++] = 0x32; // SID type

		// Payload
		ASidOutBuffer[index++] = 0; // Chip index, only one chip
		ASidOutBuffer[index++] = is6581? 0x00 : 0x01; // bits 7-1 reserved

		// Sysex end marker
		ASidOutBuffer[index++] = 0xf7;

		// Send to physical MIDI port
		if (m_RtMidiOut->isPortOpen())
			m_RtMidiOut->sendMessage(ASidOutBuffer, index);
	}
	
	void ASid::WriteToSIDRegister(unsigned char inSidReg, unsigned char inData)
	{
		if (m_Muted)
			return;
		if (inSidReg > 0x18) 
			return;

		// Get the ASID transformed register
		unsigned char MappedAddress = GetASIDPositionFromRegisterIndex(inSidReg);

		// If a write occurs to a waveform register, check if first block is already allocated
		if (MappedAddress >= 0x16 && MappedAddress <= 0x18 && m_ASIDRegisterUpdated[MappedAddress])
		{
			MappedAddress += 3;

			// If second block is also updated, move it to the first to make sure to always keep the last one
			if(m_ASIDRegisterUpdated[MappedAddress])
				m_ASIDRegisterBuffer[MappedAddress - 3] = m_ASIDRegisterBuffer[MappedAddress];
		}

		// If we're trying to update a control register that is already mapped, flush it directly
		if(m_ASIDRegisterUpdated[MappedAddress])
		{
			if(MappedAddress >= 0x16)
				SendToDevice();
		}

		// Store the data
		m_ASIDRegisterBuffer[MappedAddress] = inData;
		m_ASIDRegisterUpdated[MappedAddress] = true;
	}
	
	void ASid::SendToDevice()
	{
		if(m_Muted)
			return;
		
		const bool RequireUpdate = [&RegisterUpdated = m_ASIDRegisterUpdated]()
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
			m_ASIDOutBuffer[0] = 0xf0;
			m_ASIDOutBuffer[1] = 0x2d;
			m_ASIDOutBuffer[2] = 0x4e;
		
			size_t index = 3;

			// Setup mask bytes (one bit per register)
			unsigned char ucReg;
		
			for (unsigned char ucMask = 0; ucMask<4; ++ucMask)
			{
				ucReg = 0x00;
				for (unsigned char ucRegOffset = 0; ucRegOffset < 7; ++ucRegOffset)
				{
					if (m_ASIDRegisterUpdated[ucMask*7+ucRegOffset])
						ucReg |= (1<<ucRegOffset);
				}
				m_ASIDOutBuffer[index++] = ucReg;
			}

			// Setup the MSB bits, one per register (since MIDI only allows for 7-bit data bytes)
			for (unsigned char ucMsb=0; ucMsb<4; ++ucMsb)
			{
				ucReg = 0x00;
				for (unsigned char ucRegOffset = 0; ucRegOffset < 7; ++ucRegOffset)
				{
					if (m_ASIDRegisterBuffer[ucMsb*7 + ucRegOffset] & 0x80)
						ucReg |= (1 << ucRegOffset);
				}
				m_ASIDOutBuffer[index++] = ucReg;
			}

			// Add data for all updated registers (the 7 LSB bits)
			for (unsigned int i = 0; i < ASID_NUM_REGS; ++i)
			{
				if (m_ASIDRegisterUpdated[i])
					m_ASIDOutBuffer[index++] = m_ASIDRegisterBuffer[i] & 0x7f;
			}

			// Sysex end marker
			m_ASIDOutBuffer[index++] = 0xf7;

			// Send to physical MIDI port
			m_RtMidiOut->sendMessage(m_ASIDOutBuffer, index);
		}

		// Prepare for next buffer
		for (int i = 0; i < ASID_NUM_REGS; ++i)
			m_ASIDRegisterUpdated[i] = false;
	}

	void ASid::SendSetChannelsSilent()
	{
		for (unsigned int i = 0; i < ASID_NUM_REGS; ++i)
		{
			m_ASIDRegisterBuffer[i] = 0;
			m_ASIDRegisterUpdated[i] = false;
		}

		for(unsigned int i=0; i<3; ++i)
		{
			unsigned char channel_offset = static_cast<unsigned char>(i * 7);
				
			WriteToSIDRegister(0x04 + channel_offset, 0);
			WriteToSIDRegister(0x05 + channel_offset, 0);
			WriteToSIDRegister(0x06 + channel_offset, 0);
		}

		SendToDevice();
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
