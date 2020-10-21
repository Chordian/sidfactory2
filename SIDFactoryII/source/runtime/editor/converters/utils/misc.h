#pragma once

#include <vector>
#include "sf2_interface.h"

namespace Converter
{
	const std::string c_NotesSharp[12] =
	{
		"C-",
		"C#",
		"D-",
		"D#",
		"E-",
		"F-",
		"F#",
		"G-",
		"G#",
		"A-",
		"A#",
		"B-"
	};

	class Misc
	{
	public:

		Misc(SF2::Interface* inInterface, bool inIgnoreColors = false);
		~Misc();

		void HexBlock(unsigned int inAddress, unsigned char* inBuffer, unsigned int inBufferByteCount) const;
		void HexBlock(SF2::Interface::Block inBlock) const;

		void HexSF2Info() const;

		void HexOrderListInMemory(int inTrack) const;
		void HexSequenceInMemory(unsigned int inSequenceIndex) const;
		void HexTableInMemory(int inTableType) const;

		void HexOrderList(int inTrack) const;
		void HexSequence(int inSequenceIndex, bool InVerbose = false) const;
		void HexTable(int inTableType) const;

		void HexByte(unsigned char inByte) const;
		void HexByte(const std::vector<unsigned char>& inBytes) const;

	private:
		std::string GetHexBlock(unsigned int inAddress, unsigned char* inBuffer, unsigned int inBufferByteCount) const;

		SF2::Interface* m_SF2;

		std::string m_Zebra[2] = { "", "" };
		bool m_IgnoreColors;
	};
}