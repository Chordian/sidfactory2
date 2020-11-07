#include "c64file.h"
#include <memory>
#include <cstring>
#include "foundation/base/assert.h"

namespace Utility
{
	C64File::C64File(const void* inPRGData, unsigned int inDataSize)
		: m_Data(nullptr)
		, m_BottomAddress(0)
		, m_TopAddress(0)
	{
		FOUNDATION_ASSERT(inPRGData != nullptr);

		if (inDataSize > 2)
		{
			const unsigned char* data_bytes = static_cast<const unsigned char*>(inPRGData);
			const unsigned int top_addres = static_cast<unsigned short>(data_bytes[0]) | (static_cast<unsigned short>(data_bytes[1]) << 8);
			const unsigned int bottom_address = top_addres + inDataSize - 2;

			FOUNDATION_ASSERT(bottom_address < 0x10000);

			m_TopAddress = static_cast<unsigned short>(top_addres);
			m_BottomAddress = static_cast<unsigned short>(bottom_address);

			m_Data = new unsigned char[0x10000];
			memset(m_Data, 0, 0x10000);

			if (IsValid())
				memcpy(&m_Data[m_TopAddress], &data_bytes[2], bottom_address - top_addres);
		}
	}


	C64File::C64File(unsigned short inTopAddress, unsigned int inBottomAddress)
		: m_TopAddress(inTopAddress)
		, m_BottomAddress(inBottomAddress)
	{
		m_Data = new unsigned char[0x10000];
		memset(m_Data, 0, 0x10000);
	}


	C64File::C64File(unsigned short inTopAddress, const void* inData, unsigned short inDataSize)
		: m_TopAddress(inTopAddress)
		, m_BottomAddress(inTopAddress + inDataSize)
	{
		const unsigned char* data_bytes = static_cast<const unsigned char*>(inData);
		m_Data = new unsigned char[0x10000];

		memset(m_Data, 0, 0x10000);
		memcpy(&m_Data[m_TopAddress], data_bytes, inDataSize);
	}


	C64File::~C64File()
	{
		delete m_Data;
	}

	//----------------------------------------------------------------------------------------------------

	std::shared_ptr<C64File> C64File::CreateFromPRGData(const void* inPRGData, unsigned int inDataSize)
	{
		std::shared_ptr<C64File> c64_file = std::shared_ptr<C64File>(new C64File(inPRGData, inDataSize));
		if (c64_file->IsValid())
			return c64_file;

		return nullptr;
	}


	std::shared_ptr<C64File> C64File::CreateAsContainer(unsigned short inTopAddress, unsigned int inBottomAddres)
	{
		return std::shared_ptr<C64File>(new C64File(inTopAddress, inBottomAddres));
	}


	std::shared_ptr<C64File> C64File::CreateFromData(unsigned short inTopAddress, const void* inData, unsigned short inDataSize)
	{
		return std::shared_ptr<C64File>(new C64File(inTopAddress, inData, inDataSize));
	}


	unsigned short C64File::ReadTargetAddressFromData(const void* inData, unsigned short inDataSize)
	{
		if (inDataSize > 2)
		{
			const unsigned char* data_bytes = static_cast<const unsigned char*>(inData);
			return static_cast<unsigned short>(data_bytes[0]) | (static_cast<unsigned short>(data_bytes[1]) << 8);
		}

		return 0;
	}

	//----------------------------------------------------------------------------------------------------

	bool C64File::IsValid() const
	{
		return m_TopAddress < m_BottomAddress;
	}

	unsigned short C64File::GetTopAddress() const
	{
		return m_TopAddress;
	}


	unsigned short C64File::GetBottomAddress() const
	{
		return m_BottomAddress;
	}


	void C64File::MoveDataToTopAddress(unsigned short inTopAddress)
	{
		unsigned char* new_data = new unsigned char[0x10000];
		int data_to_move = 0x10000 - (inTopAddress > m_TopAddress ? inTopAddress : m_TopAddress);

		memset(new_data, 0, 0x10000);
		memcpy(&new_data[inTopAddress], &m_Data[m_TopAddress], data_to_move);

		delete[] m_Data;
		m_Data = new_data;

		int delta_address = inTopAddress - m_TopAddress;

		m_TopAddress += delta_address;
		m_BottomAddress += delta_address;
	}


	unsigned char* C64File::GetDataCopyAsPRG()
	{
		const int data_size = m_BottomAddress - m_TopAddress;
		unsigned char* prg_data = new unsigned char[data_size + 2];

		prg_data[0] = static_cast<unsigned char>(m_TopAddress & 0xff);
		prg_data[1] = static_cast<unsigned char>(m_TopAddress >> 8);

		memcpy(&prg_data[2], &m_Data[m_TopAddress], data_size);

		return prg_data;
	}


	unsigned int C64File::GetPRGDataSize()
	{
		return (m_BottomAddress - m_TopAddress) + 2;
	}


	//----------------------------------------------------------------------------------------------------

	unsigned char* C64File::GetData() const
	{
		return &m_Data[m_TopAddress];
	}


	unsigned int C64File::GetDataSize() const
	{
		return m_BottomAddress - m_TopAddress;
	}

	//----------------------------------------------------------------------------------------------------

	void C64File::MoveBottomAddress(unsigned short inBottomAddress)
	{
		FOUNDATION_ASSERT(inBottomAddress > GetTopAddress());

		m_BottomAddress = inBottomAddress;
	}

	//----------------------------------------------------------------------------------------------------

	unsigned char& C64File::operator[](int inAddress) 
	{
		FOUNDATION_ASSERT(m_Data != nullptr);
		FOUNDATION_ASSERT(inAddress >= m_TopAddress && inAddress < m_BottomAddress);

		return m_Data[inAddress];
	}


	const unsigned char& C64File::operator[](int inAddress) const
	{
		FOUNDATION_ASSERT(m_Data != nullptr);
		FOUNDATION_ASSERT(inAddress >= m_TopAddress && inAddress < m_BottomAddress);

		return m_Data[inAddress];
	}


	unsigned char C64File::GetByte(unsigned int inAddress) const
	{
		FOUNDATION_ASSERT(m_Data != nullptr);
		FOUNDATION_ASSERT(inAddress >= m_TopAddress && inAddress < m_BottomAddress);

		return static_cast<unsigned char>(m_Data[inAddress]);
	}


	unsigned short C64File::GetWord(unsigned int inAddress) const
	{
		FOUNDATION_ASSERT(m_Data != nullptr);
		FOUNDATION_ASSERT(inAddress >= m_TopAddress && inAddress + 1 < m_BottomAddress);

		return static_cast<unsigned short>(m_Data[inAddress]) | (static_cast<unsigned short>(m_Data[inAddress + 1]) << 8);
	}


	void C64File::GetData(unsigned int nAddress, void* pDestinationBuffer, unsigned int nDestinationBufferByteCount) const
	{
		FOUNDATION_ASSERT(pDestinationBuffer != nullptr);

		unsigned int top = nAddress;
		unsigned int bottom = nDestinationBufferByteCount;

		FOUNDATION_ASSERT(top >= m_TopAddress && top < m_BottomAddress);
		FOUNDATION_ASSERT(bottom >= m_TopAddress && bottom < m_BottomAddress);

		memcpy(pDestinationBuffer, &m_Data[top], nDestinationBufferByteCount);
	}



	//----------------------------------------------------------------------------------------------------------------------------------
	// Reader
	//----------------------------------------------------------------------------------------------------------------------------------

	C64FileReader::C64FileReader(const C64File& inFile, unsigned short inAddress)
		: m_File(inFile)
		, m_Address(inAddress)
		, m_EndAddress(inFile.GetBottomAddress())
	{
	}


	C64FileReader::C64FileReader(const C64File& inFile, unsigned short inAddress, unsigned short inEndAddress)
		: m_File(inFile)
		, m_Address(inAddress)
		, m_EndAddress(inEndAddress)
	{
		FOUNDATION_ASSERT(inEndAddress <= inFile.GetBottomAddress());
	}


	unsigned short C64FileReader::GetReadAddress() const
	{
		return m_Address;
	}


	unsigned short C64FileReader::GetEndAddress() const
	{
		return m_EndAddress;
	}


	bool C64FileReader::IsAtReadableAddress() const
	{
		return !IsAtEndAddress() && !HasPassedEnd();
	}

	bool C64FileReader::IsAtEndAddress() const
	{
		return m_Address == m_EndAddress;
	}


	bool C64FileReader::HasPassedEnd() const
	{
		return m_Address > m_EndAddress;
	}


	const unsigned char C64FileReader::ReadByte()
	{
		if (IsAtEndAddress() || HasPassedEnd())
		{
			++m_Address;
			return 0;
		}

		const unsigned char value = m_File[m_Address];
		++m_Address;

		return value;
	}


	const unsigned short C64FileReader::ReadWord()
	{
		if (IsAtEndAddress() || HasPassedEnd())
		{
			m_Address += 2;
			return 0;
		}

		const unsigned short value = m_File.GetWord(m_Address);
		m_Address += 2;

		return value;
	}


	const unsigned int C64FileReader::ReadUInt()
	{
		int value = 0;

		for (int i = 0; i < 4; i++)
		{
			value <<= 8;
			value |= ReadByte();
		}

		return value;
	}


	const int C64FileReader::ReadInt()
	{
		unsigned int value = ReadUInt();
		return static_cast<int>(value);
	}


	const std::string C64FileReader::ReadNullTerminatedString() 
	{
		unsigned short search_address = m_Address;

		// Search for null terminator
		while (m_File[search_address] != 0)
			++search_address;

		// Allocate buffer for the string
		unsigned short string_buffer_size = 1 + (search_address - m_Address);
		char* string_buffer = new char[string_buffer_size];

		// Read each byte and convert to c_str char
		for (int i = 0; i < string_buffer_size; ++i)
		{
			char value = static_cast<char>(ReadByte());
			if (value > 0 && value < 0x20)
				value += 0x60;

			string_buffer[i] = value;
		}

		// Create the string
		std::string output = std::string(string_buffer);

		// Delete the temporary buffer
		delete[] string_buffer;

		return output;
	}


	const std::vector<unsigned char> C64FileReader::ReadBytes(unsigned int inByteCount) 
	{
		std::vector<unsigned char> output;

		for (unsigned int i = 0; i < inByteCount; ++i)
		{
			unsigned char byte = ReadByte();
			output.push_back(byte);
		}

		return output;
	}



	//----------------------------------------------------------------------------------------------------------------------------------
	// Writer
	//----------------------------------------------------------------------------------------------------------------------------------

	C64FileWriter::C64FileWriter(C64File& inFile, unsigned short inAddress, bool inMayExtendFileSize)
		: m_File(inFile)
		, m_Address(inAddress)
		, m_MayExtendFileSize(inMayExtendFileSize)
	{

	}


	unsigned short C64FileWriter::GetWriteAddress() const
	{
		return m_Address;
	}

	
	unsigned short C64FileWriter::GetEndAddress() const
	{
		return m_File.GetBottomAddress();
	}

	
	bool C64FileWriter::IsAtEndAddress() const
	{
		return m_Address == GetEndAddress();
	}

	
	bool C64FileWriter::HasPassedEnd() const
	{
		return m_Address > GetEndAddress();
	}

	
	const bool C64FileWriter::WriteByte(unsigned char inByte)
	{
		FOUNDATION_ASSERT(!HasPassedEnd());
		FOUNDATION_ASSERT(m_MayExtendFileSize || !IsAtEndAddress());
		FOUNDATION_ASSERT(m_Address < 0Xffff);
		
		unsigned short new_address = m_Address + 1;
		if (new_address >= m_File.GetBottomAddress())
			m_File.MoveBottomAddress(new_address);

		m_File[m_Address] = inByte;
		++m_Address;

		return true;
	}

	
	const bool C64FileWriter::WriteWord(unsigned short inWord)
	{
		FOUNDATION_ASSERT(!HasPassedEnd());
		FOUNDATION_ASSERT(m_MayExtendFileSize || !IsAtEndAddress());
		FOUNDATION_ASSERT(m_Address < 0Xfffe);

		unsigned short new_address = m_Address + 2;
		if (new_address >= m_File.GetBottomAddress())
			m_File.MoveBottomAddress(new_address);

		m_File[m_Address + 0] = static_cast<unsigned char>(inWord & 0xff);
		m_File[m_Address + 1] = static_cast<unsigned char>((inWord >> 8) & 0xfF);

		m_Address += 2;

		return true;
	}

	
	const bool C64FileWriter::WriteUInt(unsigned int inUInt)
	{
		FOUNDATION_ASSERT(!HasPassedEnd());
		FOUNDATION_ASSERT(m_MayExtendFileSize || !IsAtEndAddress());
		FOUNDATION_ASSERT(m_Address < 0Xfffc);

		unsigned short new_address = m_Address + 4;
		if (new_address >= m_File.GetBottomAddress())
			m_File.MoveBottomAddress(new_address);

		m_File[m_Address + 0] = static_cast<unsigned char>((inUInt >> 24) & 0xff);
		m_File[m_Address + 1] = static_cast<unsigned char>((inUInt >> 16) & 0xff);
		m_File[m_Address + 2] = static_cast<unsigned char>((inUInt >> 8) & 0xff);
		m_File[m_Address + 3] = static_cast<unsigned char>((inUInt) & 0xff);

		m_Address += 4;

		return true;
	}

	
	const bool C64FileWriter::WriteInt(int inInt)
	{
		return WriteUInt(static_cast<unsigned int>(inInt));
	}


	const bool C64FileWriter::WriteNullTerminatedString(const std::string& inString)
	{
		const size_t size = inString.length();
		const char* c_str = inString.c_str();

		for (unsigned int i=0; i<size; ++i)
		{
			if (!WriteByte(c_str[i]))
				return false;
		}

		return true;
	}


	const bool C64FileWriter::WriteBytes(const std::vector<unsigned char>& inBytes)
	{
		for (const unsigned char& byte : inBytes)
			WriteByte(byte);

		return true;
	}


	const bool C64FileWriter::WriteBytes(const unsigned char* inBytes, unsigned short inByteCount)
	{
		FOUNDATION_ASSERT(inBytes != nullptr);

		for (int i = 0; i < inByteCount; ++i)
			WriteByte(inBytes[i]);

		return true;
	}
}
