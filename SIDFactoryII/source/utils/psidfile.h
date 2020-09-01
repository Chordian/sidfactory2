#pragma once

#include <string>

namespace Utility
{
	class PSIDFile final
	{
	private:
		
#pragma pack(push, 1)
		struct Header
		{
			char m_MagicNumber[4];				// 0x00
			unsigned short m_Version;			// 0x04
			unsigned short m_DataOffset;		// 0x06
			unsigned short m_LoadAddress;		// 0x08
			unsigned short m_InitAddress;		// 0x0a
			unsigned short m_UpdateAddress;		// 0x0c
			unsigned short m_SongCount;			// 0x0e
			unsigned short m_DefaultSong;		// 0x10
			unsigned int m_SpeedFlags;			// 0x12
			char m_Title[0x20];					// 0x16
			char m_Author[0x20];				// 0x36
			char m_Copyright[0x20];				// 0x56
			unsigned short m_Flags;				// 0x76
			unsigned char m_StartPage;			// 0x78
			unsigned char m_PageLength;			// 0x79
			unsigned char m_SecondSIDAddress;	// 0x7a
			unsigned char m_ThirdSIDAddress;	// 0x7a
		};
#pragma pack(pop)


	public:
		PSIDFile(
			const unsigned char* inPRGFormatedData,
			const unsigned short inDataSize,
			const unsigned short inInitOffset,
			const unsigned short inUpdateOffset,
			const unsigned short inSongCount,
			const std::string& inTitle,
			const std::string& inAuthor,
			const std::string& inCopyright,
			const bool in6581,
			const bool inPAL);

		~PSIDFile();

		const unsigned char* GetData() const;
		unsigned int GetDataSize() const;

	private:
		void CopyString(const std::string& inString, char* outCharArray);

		Header m_Header;
		unsigned int m_DataSize;
		unsigned char* m_Data;
	};
}