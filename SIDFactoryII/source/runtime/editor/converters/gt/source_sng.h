#pragma once

#include "runtime/editor/converters/utils/sf2_interface.h"

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

		SourceSng(SF2::Interface* inInterface, unsigned char* inByteBuffer);
		~SourceSng();

		bool Convert(int inTranspose);
		const std::string& GetErrorMessage() const;

	private:

		void Unsupported(const std::string& inSngCommand);
		void AddToCommandRoster(unsigned char inSngCommand);

		SF2::Interface* m_SF2;
		unsigned char* m_ByteData;

		int m_SngVersion;

		std::vector<std::string> m_SngCommandChecked;
		std::vector<std::string> m_SngCommandsUsed;

		std::string m_ErrorMessage;
	};
}