#pragma once

#include "misc.h"
#include "sf2_interface.h"

namespace Converter
{
	class SourceSng
	{
	public:

		struct SF2Event
		{
			unsigned char m_SF2Instr;
			unsigned char m_SF2Command;
			unsigned char m_SF2Note;
		};

		SourceSng(SF2::Interface* inInterface, Misc* inOutput, unsigned char* inByteBuffer);
		~SourceSng();

		void Convert(int inTranspose);

	private:

		void Unsupported(const std::string& inSngCommand);
		void AddToCommandRoster(unsigned char inSngCommand);

		SF2::Interface* m_SF2;
		Converter::Misc* m_Output;
		unsigned char* m_ByteData;

		int m_SngVersion;
		std::vector<std::string> m_SngCommandChecked;
		std::vector<std::string> m_SngCommandsUsed;
	};
}