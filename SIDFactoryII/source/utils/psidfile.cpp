#include "utils/psidfile.h"

#include <string>
#include <cstring>
#include "foundation/base/assert.h"

namespace Utility
{
	unsigned short endian_convert(unsigned short inValue)
	{
		return (inValue >> 8) | (inValue << 8);
	}

	PSIDFile::PSIDFile(
		const unsigned char* const inPRGFormatedData,
		const unsigned short inDataSize,
		const unsigned short inInitOffset,
		const unsigned short inUpdateOffset,
		const unsigned short inSongCount,
		const std::string& inTitle,
		const std::string& inAuthor,
		const std::string& inCopyright,
		const bool in6581,
		const bool inPAL,
		const unsigned int inSIDCount,
		const bool inVersion5,
		const SIDPanLayout inPanLayout,
		const SIDPanMode inPanMode,
		const std::vector<unsigned int>& inSongLengthsInMilliseconds)
	{
		memset(&m_Header, 0, sizeof(Header));

		FOUNDATION_ASSERT(inPRGFormatedData != nullptr);
		FOUNDATION_ASSERT(inDataSize > 2);

		unsigned short data_offset = 0x7c;
		unsigned short driver_address = static_cast<unsigned short>(inPRGFormatedData[0]) | (static_cast<unsigned short>(inPRGFormatedData[1]) << 8);

		m_Header.m_MagicNumber[0] = 'P';
		m_Header.m_MagicNumber[1] = 'S';
		m_Header.m_MagicNumber[2] = 'I';
		m_Header.m_MagicNumber[3] = 'D';

		// Version 3 adds the address of a second SID, version 4 of a third. Version 5 holds the SID
		// count, any number of SIDs, and the stereo panning
		const bool version_5 = inVersion5 && inSIDCount >= 2;
		const bool has_song_lengths = version_5 && inSongLengthsInMilliseconds.size() == inSongCount;

		m_Header.m_Version = endian_convert(version_5 ? 0x05 : (inSIDCount >= 3 ? 0x04 : (inSIDCount == 2 ? 0x03 : 0x02)));
		m_Header.m_DataOffset = endian_convert(data_offset);
		m_Header.m_LoadAddress = 0x0000;
		m_Header.m_InitAddress = endian_convert(driver_address + inInitOffset);
		m_Header.m_UpdateAddress = endian_convert(driver_address + inUpdateOffset);
		m_Header.m_SongCount = endian_convert(inSongCount);
		m_Header.m_DefaultSong = endian_convert(1);
		m_Header.m_SpeedFlags = 0;

		// Version 5 readers take the fields as null terminated strings that must not be empty, an empty
		// field makes them fall back to the fixed 32 byte fields. Unknown is "<?>" in the HVSC
		auto v5_string = [version_5](const std::string& inString)
		{
			return version_5 && inString.empty() ? std::string("<?>") : inString;
		};

		CopyString(v5_string(inTitle), m_Header.m_Title);
		CopyString(v5_string(inAuthor), m_Header.m_Author);
		CopyString(v5_string(inCopyright), m_Header.m_Copyright);

		unsigned short flags = (in6581 ? 0x10 : 0x20) | (inPAL ? 0x04 : 0x08);

		if (version_5)
		{
			// Bits 6-7 panning layout, bits 8-9 panning mode, all SIDs use the model of bits 4-5
			flags |= static_cast<unsigned short>(inPanLayout) << 6;
			flags |= static_cast<unsigned short>(inPanMode) << 8;

			// Address configuration 0 (step $20) from the standard start $d420: SID n at $d400 + n * $20
			m_Header.m_SecondSIDAddress = static_cast<unsigned char>(inSIDCount & 0x0f);
			m_Header.m_ThirdSIDAddress = 0x00;

			// Bit 10: a song length table of 4 bytes per song follows the C64 data
			if (has_song_lengths)
				flags |= 0x400;
		}
		else
		{
			// SID n sits at $d400 + n * $20, the header stores the middle byte of the address. Same model for all
			if (inSIDCount >= 2)
			{
				m_Header.m_SecondSIDAddress = 0x42;
				flags |= (in6581 ? 0x40 : 0x80);
			}

			if (inSIDCount >= 3)
			{
				m_Header.m_ThirdSIDAddress = 0x44;
				flags |= (in6581 ? 0x100 : 0x200);
			}
		}

		m_Header.m_Flags = endian_convert(flags);

		unsigned short header_size = sizeof(Header);

		FOUNDATION_ASSERT(header_size == data_offset);

		const unsigned int song_lengths_size = has_song_lengths ? static_cast<unsigned int>(inSongCount) * 4 : 0;

		m_DataSize = header_size + inDataSize + song_lengths_size;
		m_Data = new unsigned char[m_DataSize];

		memcpy(m_Data, &m_Header, sizeof(Header));
		memcpy(m_Data + data_offset, inPRGFormatedData, inDataSize);

		// Per song a BCD word MM:SS and a BCD word with the milliseconds, at the very end of the file
		if (has_song_lengths)
		{
			auto bcd = [](unsigned int inValue)
			{
				return static_cast<unsigned char>(((inValue / 10) % 10) << 4 | (inValue % 10));
			};

			unsigned char* song_lengths = m_Data + header_size + inDataSize;

			for (unsigned int i = 0; i < inSongCount; ++i)
			{
				// 99:59.999 is the longest time the table holds
				const unsigned int length = inSongLengthsInMilliseconds[i] < 6000000 ? inSongLengthsInMilliseconds[i] : 5999999;
				const unsigned int milliseconds = length % 1000;

				song_lengths[i * 4 + 0] = bcd(length / 60000);
				song_lengths[i * 4 + 1] = bcd((length / 1000) % 60);
				song_lengths[i * 4 + 2] = static_cast<unsigned char>(milliseconds / 100);
				song_lengths[i * 4 + 3] = bcd(milliseconds % 100);
			}
		}
	}


	PSIDFile::~PSIDFile()
	{
		delete m_Data;
	}


	const unsigned char* PSIDFile::GetData() const
	{
		return m_Data;
	}


	unsigned int PSIDFile::GetDataSize() const
	{
		return m_DataSize;
	}


	void PSIDFile::CopyString(const std::string& inString, char* outCharArray)
	{
		const char* string = inString.c_str();
		size_t string_length = inString.length();

		for (size_t i = 0; i < 0x20; ++i)
			outCharArray[i] = i < string_length ? string[i] : 0;
	}
}
