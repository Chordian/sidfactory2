#pragma once

#include "runtime/editor/converters/utils/sf2_interface.h"

namespace Converter
{
	class SourceMod
	{
	public:

		struct ModEvent
		{
			unsigned char m_ModNote;
			unsigned char m_ModInstr;
			unsigned char m_ModCommand;
			unsigned char m_ModData;
		};

		struct SF2Event
		{
			unsigned char m_SF2Instr;
			unsigned char m_SF2Command;
			unsigned char m_SF2Note;
		};

		SourceMod(SF2::Interface* inInterface, unsigned char* inByteBuffer);
		~SourceMod();

		bool Convert(int inLeaveout, int inTranspose);

	private:

		void Unsupported(const std::string& inModCommand);
		void AddToCommandRoster(unsigned char inModCommand);

		SF2::Interface* m_SF2;
		unsigned char* m_ByteData;

		unsigned char m_ModHeader[1084];
		unsigned char m_ModPatterns[64 * 64 * 4 * 4];
		ModEvent m_ModEvents[64 * 64 * 4];

		int m_ModMaxPattern;
		int m_ModOrderListLength;
		std::vector<std::string> m_ModCommandChecked;
		std::vector<std::string> m_ModCommandsUsed;
	};
}