#include "utils/psidfile.h"

#include <string>
#include <cstring>
#include "assert.h"

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
		const bool inPAL)
	{
		memset(&m_Header, 0, sizeof(Header));

		assert(inPRGFormatedData != nullptr);
		assert(inDataSize > 2);

		unsigned short data_offset = 0x7c;
		unsigned short driver_address = static_cast<unsigned short>(inPRGFormatedData[0]) | (static_cast<unsigned short>(inPRGFormatedData[1]) << 8);

		m_Header.m_MagicNumber[0] = 'P';
		m_Header.m_MagicNumber[1] = 'S';
		m_Header.m_MagicNumber[2] = 'I';
		m_Header.m_MagicNumber[3] = 'D';

		m_Header.m_Version = endian_convert(0x02);
		m_Header.m_DataOffset = endian_convert(data_offset);
		m_Header.m_LoadAddress = 0x0000;
		m_Header.m_InitAddress = endian_convert(driver_address + inInitOffset);
		m_Header.m_UpdateAddress = endian_convert(driver_address + inUpdateOffset);
		m_Header.m_SongCount = endian_convert(inSongCount);
		m_Header.m_DefaultSong = endian_convert(1);
		m_Header.m_SpeedFlags = 0;

		CopyString(inTitle, m_Header.m_Title);
		CopyString(inAuthor, m_Header.m_Author);
		CopyString(inCopyright, m_Header.m_Copyright);

		m_Header.m_Flags = endian_convert((in6581 ? 0x10 : 0x20) | (inPAL ? 0x04 : 0x08));

		unsigned short header_size = sizeof(Header);

		assert(header_size == data_offset);

		m_DataSize = header_size + inDataSize;
		m_Data = new unsigned char[m_DataSize];

		memcpy(m_Data, &m_Header, sizeof(Header));
		memcpy(m_Data + data_offset, inPRGFormatedData, inDataSize);
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
