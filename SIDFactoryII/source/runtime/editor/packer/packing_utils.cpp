#include "packing_utils.h"
#include "runtime/emulation/cpumos6510.h"

namespace Editor
{
	ZeroPageRange GetZeroPageRangeFromDriver(Emulation::CPUMemory& inCPUMemory, const DriverInfo& inDriverInfo)
	{
		auto is_zeropage = [](Emulation::CPUmos6510::AddressingMode inAddressingMode)
		{
			switch (inAddressingMode)
			{
			case Emulation::CPUmos6510::am_ZP:
			case Emulation::CPUmos6510::am_ZPX:
			case Emulation::CPUmos6510::am_ZPY:
			case Emulation::CPUmos6510::am_IZX:
			case Emulation::CPUmos6510::am_IZY:
				return true;
			default:
				break;
			}

			return false;
		};

		const int top_address = inDriverInfo.GetDescriptor().m_DriverCodeTop;
		const int bottom_address = top_address + inDriverInfo.GetDescriptor().m_DriverCodeSize;

		int address = top_address;

		unsigned char zp_lowest = 0xff;
		unsigned char zp_highest = 0x00;

		inCPUMemory.Lock();

		while (address < bottom_address)
		{
			const unsigned char opcode = inCPUMemory[address];
			const unsigned char opcode_size = Emulation::CPUmos6510::GetOpcodeByteSize(opcode);
			const Emulation::CPUmos6510::AddressingMode opcode_addressing_mode = Emulation::CPUmos6510::GetOpcodeAddressingMode(opcode);

			if (is_zeropage(opcode_addressing_mode))
			{
				FOUNDATION_ASSERT(opcode_size == 2);

				const unsigned char zp = inCPUMemory[address + 1];
				if (zp < zp_lowest)
					zp_lowest = zp;
				if (zp > zp_highest)
					zp_highest = zp;
			}

			address += static_cast<unsigned short>(opcode_size);
		}

		inCPUMemory.Unlock();

		if (zp_lowest < zp_highest)
			return { zp_lowest, zp_highest };

		return { 0, 0 };
	}
}