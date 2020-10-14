#include <iostream>

#include "source_utils.h"
#include "utils/utilities.h"
#include <cstring>

namespace Converter
{
	SourceUtils::SourceUtils(SF2::Interface* inInterface, Misc* inOutput)
		: m_SF2(inInterface)
		, m_Output(inOutput)
		, m_SourceData(nullptr)
		, m_ByteData(nullptr)
		, m_ByteDataSize(0)
		, m_SourceType(SourceType::unknown)
		, m_SourceIdentified(false)
		, m_SourceMod(nullptr)
		, m_SourceSng(nullptr)
		, m_SourceCt(nullptr)
	{
	}

	SourceUtils::~SourceUtils()
	{
		delete m_SourceData;
		delete m_SourceMod;
		delete m_SourceSng;
	}

	/**
	 * Load the source file and return a byte-sized pointer.
	 */
	unsigned char* SourceUtils::LoadFile(const std::string& inSourceFilename)
	{
		bool file_read = Utility::ReadFile(inSourceFilename, 0, &m_SourceData, m_ByteDataSize);
		if (!file_read)
		{
			std::cerr << "ERROR: Could not load \"" << inSourceFilename << "\" file." << std::endl;
			exit(0);
		}

		m_ByteData = static_cast<unsigned char*>(m_SourceData);
		return m_ByteData;
	}

	/**
	 * Save the finalized SF2 source file.
	 */
	void SourceUtils::SaveFile(const std::string& inDestinationFilename)
	{
		bool file_saved = m_SF2->SaveFile(inDestinationFilename);
		if (!file_saved)
		{
			std::cerr << "\nERROR: Could not save \"" << inDestinationFilename << "\" file." << std::endl;
			exit(0);
		}
		else
			std::cout << "\nSaved \"" << inDestinationFilename << "\" file." << std::endl;
	}

	/**
	 * Try to identify the type of source file.
	 */
	int SourceUtils::Identify()
	{
		// Letters that are often present in MOD files
		// @link http://fileformats.archiveteam.org/wiki/Amiga_Module
		if (memcmp("M.K.", &m_ByteData[1080], 4) == 0 || memcmp("FLT", &m_ByteData[1080], 3) == 0)
			m_SourceType = SourceType::mod;	// MOD file
		else if (memcmp("GTS3", &m_ByteData[0], 4) == 0 || memcmp("GTS4", &m_ByteData[0], 4) == 0 || memcmp("GTS5", &m_ByteData[0], 4) == 0)
			m_SourceType = SourceType::sng;	// GoatTracker 2
		else if (memcmp("CC2", &m_ByteData[0], 3) == 0)
			m_SourceType = SourceType::ct;	// CheeseCutter

		m_SourceIdentified = true;

		return m_SourceType;
	}

	/**
	 * Convert from the source file to SF2.
	 */
	bool SourceUtils::Convert(int inLeaveout, int inTranspose)
	{
		if (!m_SourceIdentified)
			Identify();

		switch (m_SourceType)
		{
		case(SourceType::mod):
			// Converting from MOD
			std::cout << "  MOD format detected.\n" << std::endl;

			m_SourceMod = new SourceMod(m_SF2, m_Output, m_ByteData);
			m_SourceMod->Convert(inLeaveout, inTranspose);
			break;

		case(SourceType::sng):
			// Converting from SNG (GoatTracker 2)
			std::cout << "  SNG format from GoatTracker 2 detected.\n" << std::endl;

			m_SourceSng = new SourceSng(m_SF2, m_Output, m_ByteData);
			m_SourceSng->Convert(inTranspose);
			break;

		case(SourceType::ct):
			// Converting from CT (CheeseCutter)
			std::cout << "  CT format from CheeseCutter detected.\n" << std::endl;

			m_SourceCt = new SourceCt(m_SF2, m_Output, m_ByteData, m_ByteDataSize);
			m_SourceCt->Convert(inTranspose);
			break;

		default:
			std::cerr << "ERROR: Unidentified source file format." << std::endl;
			exit(0);
		}

		return true;
	}
}
