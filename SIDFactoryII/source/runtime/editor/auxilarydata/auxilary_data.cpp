#include "auxilary_data.h"
#include "utils/c64file.h"

#include <assert.h>


namespace Editor
{
	// FileHeader
	//-----------------

	const AuxilaryData::FileHeader AuxilaryData::FileHeader::Read(Utility::C64FileReader& inReader)
	{
		const Type data_type = static_cast<Type>(inReader.ReadByte());
		const unsigned short data_version = inReader.ReadWord();
		const unsigned short data_size = inReader.ReadWord();

		return FileHeader(data_type, data_version, data_size);
	}


	void AuxilaryData::FileHeader::Write(Utility::C64FileWriter& inWriter) const
	{
		inWriter.WriteByte(static_cast<unsigned char>(m_DataType));
		inWriter.WriteWord(m_DataVersion);
		inWriter.WriteWord(m_DataSize);
	}


	const AuxilaryData::Type AuxilaryData::FileHeader::GetDataType() const
	{
		return m_DataType;
	}


	const unsigned short AuxilaryData::FileHeader::GetDataVersion() const
	{
		return m_DataVersion;
	}


	const unsigned short AuxilaryData::FileHeader::GetDataSize() const
	{
		return m_DataSize;
	}



	// Song Data Base
	//------------------

	AuxilaryData::AuxilaryData(AuxilaryData::Type inType)
		: m_DataType(inType)
	{
	}

	
	AuxilaryData::AuxilaryData()
		: m_DataType(Type::Undefined)
	{
	}

	
	AuxilaryData::~AuxilaryData()
	{
	}


	void AuxilaryData::Reset()
	{
		assert(false);
	}


	bool AuxilaryData::Read(const FileHeader& inHeader, Utility::C64FileReader& inReader)
	{
		if (IsMyHeader(inHeader))
		{
			std::vector<unsigned char> data = inReader.ReadBytes(inHeader.GetDataSize());
			return RestoreFromSaveData(inHeader.GetDataVersion(), data);
		}

		return false;
	}


	bool AuxilaryData::Write(Utility::C64FileWriter& inWriter) const
	{
		std::vector<unsigned char> data = GenerateSaveData();
		FileHeader file_header = GenerateFileHeader(GetGeneratedFileVersion(), data);

		file_header.Write(inWriter);
		return inWriter.WriteBytes(data);
	}


	AuxilaryData::FileHeader AuxilaryData::GenerateFileHeader(unsigned short inDataVersion, const std::vector<unsigned char>& inData) const
	{
		return FileHeader(m_DataType, inDataVersion, static_cast<unsigned short>(inData.size()));
	}


	unsigned short AuxilaryData::GetGeneratedFileVersion() const
	{
		return 0;
	}


	bool AuxilaryData::IsMyHeader(const FileHeader& inHeader) const
	{
		return inHeader.GetDataType() == m_DataType;
	}



	std::vector<unsigned char> AuxilaryData::GenerateSaveData() const
	{
		return std::vector<unsigned char>();
	}


	bool AuxilaryData::RestoreFromSaveData(unsigned short inDataVersion, std::vector<unsigned char> inData)
	{
		assert(inDataVersion == 0);
		return true;
	}
}