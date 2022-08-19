#include "cpumemory.h"

#include "foundation/base/assert.h"
#include <string.h>

namespace Emulation
{
	CPUMemory::CPUMemory(unsigned int nSize, Foundation::IPlatform* inPlatform)
		: m_nSize(nSize)
		, m_IsLocked(false)
		, m_MemorySnapshot(nullptr)
	{
		FOUNDATION_ASSERT(inPlatform != nullptr);

		m_Memory = new unsigned char[nSize];
		m_Mutex = inPlatform->CreateMutex();

		memset(m_Memory, 0, nSize);
	}

	CPUMemory::~CPUMemory()
	{
		m_Mutex = nullptr;

		delete m_Memory;
		delete m_MemorySnapshot;
	}

	//------------------------------------------------------------------------------------------------------------------------------

	void CPUMemory::Lock()
	{
		m_Mutex->Lock();
		m_IsLocked = true;
	}

	void CPUMemory::Unlock()
	{
		m_IsLocked = false;
		m_Mutex->Unlock();
	}

	bool CPUMemory::IsLocked() const
	{
		return m_IsLocked;
	}

	//------------------------------------------------------------------------------------------------------------------------------

	void CPUMemory::Clear()
	{
		FOUNDATION_ASSERT(m_Memory != nullptr);
		memset(m_Memory, 0, m_nSize);
	}

	//------------------------------------------------------------------------------------------------------------------------------

	void CPUMemory::TakeSnapshot()
	{
		FOUNDATION_ASSERT(m_Memory != nullptr);
		FOUNDATION_ASSERT(m_MemorySnapshot == nullptr);
		FOUNDATION_ASSERT(m_IsLocked);

		m_MemorySnapshot = new unsigned char[m_nSize];
		memcpy(m_MemorySnapshot, m_Memory, m_nSize);
	}

	void CPUMemory::RestoreFromSnapshot()
	{
		FOUNDATION_ASSERT(m_Memory != nullptr);
		FOUNDATION_ASSERT(m_MemorySnapshot != nullptr);
		FOUNDATION_ASSERT(m_IsLocked);

		memcpy(m_Memory, m_MemorySnapshot, m_nSize);
	}

	void CPUMemory::FlushSnapshot()
	{
		FOUNDATION_ASSERT(m_MemorySnapshot != nullptr);
		FOUNDATION_ASSERT(m_IsLocked);

		delete[] m_MemorySnapshot;
		m_MemorySnapshot = nullptr;
	}

	//------------------------------------------------------------------------------------------------------------------------------

	unsigned char CPUMemory::GetByte(unsigned int nAddress) const
	{
		FOUNDATION_ASSERT(m_Memory != nullptr);
		FOUNDATION_ASSERT(nAddress < m_nSize);
		FOUNDATION_ASSERT(m_IsLocked);

		return m_Memory[nAddress];
	}

	unsigned short CPUMemory::GetWord(unsigned int nAddress) const
	{
		FOUNDATION_ASSERT(m_Memory != nullptr);
		FOUNDATION_ASSERT(nAddress < m_nSize - 1);
		FOUNDATION_ASSERT(m_IsLocked);

		return (unsigned short)m_Memory[nAddress] | (((unsigned short)m_Memory[nAddress + 1]) << 8);
	}

	void CPUMemory::GetData(unsigned int inAddress, void* inDestinationBuffer, unsigned int inDestinationBufferByteCount) const
	{
		FOUNDATION_ASSERT(inDestinationBuffer != nullptr);
		FOUNDATION_ASSERT(m_Memory != nullptr);
		FOUNDATION_ASSERT(inAddress < m_nSize + inDestinationBufferByteCount);
		FOUNDATION_ASSERT(m_IsLocked);

		unsigned char* pDest = (unsigned char*)inDestinationBuffer;

		for (unsigned int i = 0; i < inDestinationBufferByteCount; i++)
			pDest[i] = m_Memory[inAddress + i];
	}

	//------------------------------------------------------------------------------------------------------------------------------

	void CPUMemory::SetByte(unsigned int nAddress, unsigned char ucByte)
	{
		FOUNDATION_ASSERT(m_Memory != nullptr);
		FOUNDATION_ASSERT(nAddress < m_nSize);
		FOUNDATION_ASSERT(m_IsLocked);

		m_Memory[nAddress] = ucByte;
	}

	void CPUMemory::SetWord(unsigned int nAddress, unsigned short usWord)
	{
		FOUNDATION_ASSERT(m_Memory != nullptr);
		FOUNDATION_ASSERT(nAddress < m_nSize - 1);
		FOUNDATION_ASSERT(m_IsLocked);

		m_Memory[nAddress] = (unsigned char)(usWord & 0x00ff);
		m_Memory[nAddress + 1] = (unsigned char)((usWord & 0xff00) >> 8);
	}

	void CPUMemory::SetData(unsigned int nAddress, const void* pSourceBuffer, unsigned int nSourceBufferByteCount)
	{
		FOUNDATION_ASSERT(pSourceBuffer != nullptr);
		FOUNDATION_ASSERT(m_Memory != nullptr);
		FOUNDATION_ASSERT(nAddress + nSourceBufferByteCount <= m_nSize);
		FOUNDATION_ASSERT(m_IsLocked);

		unsigned char* pSrc = (unsigned char*)pSourceBuffer;

		for (unsigned int i = 0; i < nSourceBufferByteCount; i++)
			m_Memory[nAddress + i] = pSrc[i];
	}

	void CPUMemory::Copy(unsigned int inSourceAddress, unsigned int inLength, unsigned int inDestinationAddress)
	{
		FOUNDATION_ASSERT(inSourceAddress < m_nSize);
		FOUNDATION_ASSERT(inSourceAddress + inLength <= m_nSize);
		FOUNDATION_ASSERT(inDestinationAddress < m_nSize);
		FOUNDATION_ASSERT(inDestinationAddress + inLength <= m_nSize);

		memcpy(&m_Memory[inDestinationAddress], &m_Memory[inSourceAddress], inLength);
	}


	void CPUMemory::Set(unsigned char inValue, unsigned int inAddress, unsigned int inLength)
	{
		FOUNDATION_ASSERT(inAddress < m_nSize);
		FOUNDATION_ASSERT(inAddress + inLength < m_nSize);

		for (unsigned int i = 0; i < inLength; ++i)
			m_Memory[inAddress + i] = inValue;
	}
}