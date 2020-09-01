#pragma once

#include "runtime/emulation/imemoryrandomreadaccess.h"
#include <string>
#include <memory>
#include <vector>

namespace Utility
{
	class C64File final : public Emulation::IMemoryRandomReadAccess
	{
		friend class shared_ptr;

	private:
		C64File() = delete;

		C64File(const void* inPRGData, unsigned int inDataSize);
		C64File(unsigned short inTopAddress, unsigned int inBottomAddres);
		C64File(unsigned short inTopAddress, const void* inData, unsigned short inDataSize);

	public:
		C64File(const C64File&) = delete;
		~C64File();

		static std::shared_ptr<C64File> CreateFromPRGData(const void* inPRGData, unsigned int inDataSize);
		static std::shared_ptr<C64File> CreateAsContainer(unsigned short inTopAddress, unsigned int inBottomAddres);
		static std::shared_ptr<C64File> CreateFromData(unsigned short inTopAddress, const void* inData, unsigned short inDataSize);

		bool IsValid() const;

		unsigned short GetTopAddress() const;
		unsigned short GetBottomAddress() const;

		void MoveDataToTopAddress(unsigned short inTopAddress);
		unsigned char* GetDataCopyAsPRG();
		unsigned int GetPRGDataSize();

		unsigned char& operator[](int inMemoryAddress);

		// Read access
		const unsigned char& operator[](int inMemoryAddress) const override;

		unsigned char GetByte(unsigned int inAddress) const override;
		unsigned short GetWord(unsigned int inAddress) const override;
		void GetData(unsigned int inAddress, void* outDestinationBuffer, unsigned int inDestinationBufferByteCount) const override;

		unsigned char* GetData() const;
		unsigned int GetDataSize() const;

		void MoveBottomAddress(unsigned short inBottomAddress);

	private:
		unsigned short m_TopAddress;
		unsigned short m_BottomAddress;
		unsigned char* m_Data;
	};


	class C64FileReader final
	{
	public:
		C64FileReader(const C64File& inFile, unsigned short inAddress);
		C64FileReader(const C64File& inFile, unsigned short inAddress, unsigned short inEndAddress);

		unsigned short GetReadAddress() const;
		unsigned short GetEndAddress() const;

		bool IsAtReadableAddress() const;
		bool IsAtEndAddress() const;
		bool HasPassedEnd() const;

		const unsigned char ReadByte();
		const unsigned short ReadWord();
		const unsigned int ReadUInt();
		const int ReadInt();
		const std::string ReadNullTerminatedString();
		const std::vector<unsigned char> ReadBytes(unsigned int inByteCount);


	private:
		const C64File& m_File;

		unsigned short m_Address;
		unsigned short m_EndAddress;
	};


	class C64FileWriter final
	{
	public:
		C64FileWriter(C64File& inFile, unsigned short inAddress, bool inMayExtendFileSize);

		unsigned short GetWriteAddress() const;
		unsigned short GetEndAddress() const;

		bool IsAtEndAddress() const;
		bool HasPassedEnd() const;

		const bool WriteByte(unsigned char inByte);
		const bool WriteWord(unsigned short inWord);
		const bool WriteUInt(unsigned int inUInt);
		const bool WriteInt(int inInt);
		const bool WriteNullTerminatedString(const std::string& inString);
		const bool WriteBytes(const std::vector<unsigned char>& inByteArray);
		const bool WriteBytes(const unsigned char* inBytes, unsigned short inByteCount);

	private:
		C64File& m_File;

		const bool m_MayExtendFileSize;
		unsigned short m_Address;
	};
}
