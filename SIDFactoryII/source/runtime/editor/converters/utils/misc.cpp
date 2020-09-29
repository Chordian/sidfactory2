#include <sstream>
#include <iostream>
#include <iomanip>
#include <assert.h>
#include <vector>

#include "misc.h"

#define ROWSIZE 0x20

namespace Converter
{
	Misc::Misc(SF2::Interface* inInterface, bool inIgnoreColors)
		: m_SF2(inInterface)
		, m_IgnoreColors(inIgnoreColors)
		, m_Zebra()
	{
		// Colors only work in Visual Studio's console
		if (!m_IgnoreColors)
			std::string m_Zebra[2] =
			{
				"\033[0m",				// Reset (default gray)
				"\033[1m\033[37m"		// White
			};
	}

	Misc::~Misc()
	{
	}

	/**
	 * Return a string for cout with a monitor-view of a memory block.
	 */
	std::string Misc::GetHexBlock(unsigned int inAddress, unsigned char* inBuffer, unsigned int inBufferByteCount) const
	{
		std::stringstream stream;
		stream << std::setfill('0');

		unsigned int line = inAddress, color = 0;
		unsigned char adapted_char;

		while (line < inAddress + inBufferByteCount)
		{
			stream << m_Zebra[1] << std::hex << std::setw(4) << (int)line << ": " + m_Zebra[0];

			for (unsigned int byte = line, pos = 0; pos < ROWSIZE; byte++, pos++)
			{
				if (!(pos % 4)) {
					stream << " " << m_Zebra[color];
					color ^= 1;
				}
				if (byte >= inAddress + inBufferByteCount)
					stream << "   ";
				else
					stream << std::hex << std::setw(2) << (int)inBuffer[byte] << " ";
			}

			stream << " ";

			for (unsigned int byte = line, pos = 0; pos < ROWSIZE; byte++, pos++)
			{
				if (byte >= inAddress + inBufferByteCount)
					break;
				if (!(pos % 4)) {
					stream << m_Zebra[color];
					color ^= 1;
				}
				adapted_char = inBuffer[byte] < 64 ? inBuffer[byte] + 64 : inBuffer[byte];
				adapted_char = adapted_char == 64 ? 46 : adapted_char;
				stream << adapted_char;
			}

			line += ROWSIZE;
			stream << m_Zebra[0] << std::endl;
		}

		return stream.str();
	}

	/**
	 * COUT a monitor-view of a memory block.
	 */
	void Misc::HexBlock(unsigned int inAddress, unsigned char* inBuffer, unsigned int inBufferByteCount) const
	{
		std::cout << GetHexBlock(inAddress, inBuffer, inBufferByteCount) << std::endl;
	}

	void Misc::HexBlock(SF2::Interface::Block inBlock) const
	{
		std::cout << inBlock.sh_Description << std::endl << std::string(inBlock.sh_Description.length(), '-') << std::endl
			 << GetHexBlock(inBlock.sh_Address, inBlock.sh_Buffer, inBlock.sh_Count) << std::endl;
	}

	/**
	 * COUT the address range and size of the SF2 file data.
	 */
	void Misc::HexSF2Info() const
	{
		SF2::Interface::Range range = m_SF2->GetRangeSF2();
		int size = range.m_EndAddress - range.m_StartAddress;

		std::cout
			<< "Driver details\n--------------" << std::endl << std::setfill('0')
			<< "C64 memory start:  " << std::hex << std::setw(4) << range.m_StartAddress << std::endl
			<< "C64 memory end:    " << std::hex << range.m_EndAddress << std::endl
			<< "C64 memory size:   " << std::hex << size << " (" << std::dec << size  << ") " << std::endl
			<< std::endl;
	}

	/**
	 * COUT a monitor-view of an order list in C64 memory.
	 */
	void Misc::HexOrderListInMemory(int inTrack) const
	{
		HexBlock(m_SF2->GetRangeOrderList(inTrack));
	}

	/**
	 * COUT a monitor-view of a sequence in C64 memory.
	 */
	void Misc::HexSequenceInMemory(unsigned int inSequenceIndex) const
	{
		HexBlock(m_SF2->GetRangeSequence(inSequenceIndex));
	}

	/**
	 * COUT a monitor-view of a table in C64 memory.
	 */
	void Misc::HexTableInMemory(int inTableType) const
	{
		HexBlock(m_SF2->GetRangeTable(inTableType));
	}

	/**
	 * COUT the order list container for the specified track.
	 */
	void Misc::HexOrderList(int inTrack) const
	{
		std::vector<unsigned char> vec_order_list = m_SF2->GetContainerOrderList(inTrack);

		std::cout << std::setfill('0') << "Track " << (int)*vec_order_list.begin() << " order list" << "\n------------------"  << std::endl;
		for (std::vector<unsigned char>::iterator it = vec_order_list.begin() + 1; it != vec_order_list.end(); ++it)
			std::cout << std::hex << std::setw(2) << (int)*it << " ";
		std::cout << std::endl << std::endl;
	}

	/**
	 * COUT the sequence container for the specified index.
	 */
	void Misc::HexSequence(int inSequenceIndex, bool inVerbose) const
	{
		std::vector<unsigned char> vec_sequence = m_SF2->GetContainerSequence(inSequenceIndex);

		std::cout << std::setfill('0') << "Sequence #" << std::hex << std::setw(2) << (int)*vec_sequence.begin() << "\n------------" << std::endl;
		for (std::vector<unsigned char>::iterator it = vec_sequence.begin() + 1; it != vec_sequence.end(); ++it)
		{
			switch (distance(vec_sequence.begin(), it - 1) % 3)
			{
			case 0:
				if (inVerbose) std::cout << "Instr ";
				if (*it == 0x80)
					std::cout << "--";
				else if (*it == 0x90)
					std::cout << m_Zebra[1] << "**" << m_Zebra[0]; // Stars are too dark
				else
					std::cout << std::hex << std::setw(2) << (int)*it - 0xa0;
				break;
			case 1:
				if (inVerbose) std::cout << "Cmd ";
				if (*it == 0x80)
					std::cout << "--";
				else
					std::cout << std::hex << std::setw(2) << (int)*it - 0xc0;
				break;
			case 2:
				if (inVerbose) std::cout << "Note ";
				if (*it == 0x00)
					std::cout << "---";
				else if (*it == 0x7e)
					std::cout << "+++";
				else
					std::cout << c_NotesSharp[(int)*it % 12] << (int)*it / 12;
				break;
			}
			if (inVerbose)
				std::cout << " (" << std::hex << std::setw(2) << (int)*it << ") ";
			std::cout << (distance(vec_sequence.begin(), it - 1) % 3 == 2 ? "\n" : " ");
		}
		std::cout << std::endl;
	}

	/**
	 * COUT the specified table.
	 */
	void Misc::HexTable(int inTableType) const
	{
		SF2::Interface::Table table = m_SF2->GetTable(inTableType);

		if (table.m_Name == "")
		{
			std::cerr << "ERROR: Unknown table specified." << std::endl;
			return;
		}

		std::cout << std::setfill('0') << table.m_Name << std::endl << std::string(table.m_Name.length(), '-') << std::endl;
		std::vector<unsigned char>::iterator it = table.m_Data.begin();
		for (int row = 0; row < table.m_RowCount; row++) {
			std::cout << m_Zebra[1] << std::hex << std::setw(2) << row << ": " << m_Zebra[0];
			for (int col = 0; col < table.m_ColumnCount; col++)
			{
				std::cout << std::hex << std::setw(2) << (int)*it++ << " ";
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}

	/**
	 * COUT one byte. Useful for lazy debugging.
	 */
	void Misc::HexByte(unsigned char inByte) const
	{
		std::cout << std::setfill('0') << std::setw(2) << std::hex << (int)inByte << " ";
	}

	/**
	 * COUT several bytes. Useful for lazy debugging.
	 */
	void Misc::HexByte(const std::vector<unsigned char>& inBytes) const
	{
		std::cout << std::setfill('0');
		for (unsigned char byte : inBytes)
			std::cout << std::setw(2) << std::hex << (int)byte << " ";
		std::cout << std::endl;
	}
}