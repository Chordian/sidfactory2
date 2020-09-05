#pragma once

#include "sf2_interface.h"
#include "source_mod.h"
#include "source_sng.h"
#include "source_ct.h"

namespace Converter
{
	class SourceUtils
	{
	public:

		enum SourceType : int
		{
			unknown,
			mod,
			sng,
			ct
		};

		SourceUtils(SF2::Interface* inInterface, Misc* inOutput);
		~SourceUtils();

		unsigned char* LoadFile(const std::string& inPathAndFilename);
		void SaveFile(const std::string& inDestinationFilename);
		int Identify();
		bool Convert(int inLeaveout, int inTranspose);

	private:

		SourceMod* m_SourceMod;
		SourceSng* m_SourceSng;
		SourceCt* m_SourceCt;

		SF2::Interface* m_SF2;
		Converter::Misc* m_Output;

		int m_SourceType;
		bool m_SourceIdentified;

		void* m_SourceData;
		unsigned char* m_ByteData;
		long m_ByteDataSize;
	};
}