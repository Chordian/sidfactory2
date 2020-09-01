#pragma once

#include <memory>
#include <vector>

namespace Utility
{
	class C64FileReader;
	class C64FileWriter;
}

namespace Editor
{
	#pragma warning(disable: 4100)

	class AuxilaryData
	{
	public:
		enum Type : unsigned char
		{
			Undefined,
			EditingPreferences,
			HardwarePreferences,
			PlayMarkers,
			TableText
		};

		class FileHeader
		{
		public:
			FileHeader(const Type inType, const short inDataVersion, const unsigned short inDataSize)
				: m_DataType(inType)
				, m_DataVersion(inDataVersion)
				, m_DataSize(inDataSize)
			{
			}

			static const FileHeader Read(Utility::C64FileReader& inReader);
			void Write(Utility::C64FileWriter& inWriter) const;

			const Type GetDataType() const;
			const unsigned short GetDataVersion() const;
			const unsigned short GetDataSize() const;

		private:

			const Type m_DataType;
			const unsigned short m_DataVersion;
			const unsigned short m_DataSize;
		};

	protected:
		AuxilaryData(Type inDataType);
	public:
		AuxilaryData();
		~AuxilaryData();

		virtual void Reset();

		bool Write(Utility::C64FileWriter& inWriter) const;
		bool Read(const FileHeader& inHeader, Utility::C64FileReader& inReader);

	protected:
		FileHeader GenerateFileHeader(unsigned short inDataVersion, const std::vector<unsigned char>& inSaveData) const;
		bool IsMyHeader(const FileHeader& inHeader) const;

		virtual std::vector<unsigned char> GenerateSaveData() const;		
		virtual unsigned short GetGeneratedFileVersion() const;

		virtual bool RestoreFromSaveData(unsigned short inDataVersion, std::vector<unsigned char> inData);

		Type m_DataType;
	};
}