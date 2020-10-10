#pragma once

#include "runtime/editor/converters/utils/sf2_interface.h"

#define MINIZ_HEADER_FILE_ONLY
#include "libraries/miniz/miniz.c" // Handles zlib uncompress()

namespace Converter
{
	class SourceCt
	{
	public:

		struct SF2Event
		{
			unsigned char m_SF2Instr;
			unsigned char m_SF2Command;
			unsigned char m_SF2Note;
		};

		SourceCt(SF2::Interface* inInterface, unsigned char* inByteBuffer, long inByteBufferSize);
		~SourceCt();

		bool CanConvert() const; 
		bool Convert(int inTranspose);

	private:

		void Unsupported(const std::string& inCtCommand);
		void AddToCommandRoster(unsigned char inCtCommand);

		unsigned char HandlePulseProgram(unsigned char inIndex);
		unsigned char HandleFilterProgram(unsigned char inIndex);

		SF2::Interface* m_SF2;
		unsigned char* m_ByteData;
		long m_ByteDataSize;

		unsigned char* m_RawData;
		unsigned long m_RawDataSize;

		int m_CtVersion;
		std::vector<std::string> m_CtCommandChecked;
		std::vector<std::string> m_CtCommandsUsed;

		unsigned char m_JumpIndex;
		unsigned char m_Instrument[8];
		unsigned char m_PulseTable[0x40][4];
		unsigned char m_FilterTable[0x40][4];
		std::vector<std::vector<unsigned char>> m_Cluster;
		bool m_WarningShownPulseScope;
		bool m_WarningShownFilterScope;
		bool m_UnpackingSucceeded;
	};
}