#include "cpumemory.h"

#include <assert.h>
#include <string.h>

namespace Emulation
{
	CPUMemory::CPUMemory(unsigned int nSize, Foundation::IPlatform* inPlatform)
		: m_nSize(nSize)
		, m_IsLocked(false)
		, m_MemorySnapshot(nullptr)
	{
		assert(inPlatform != nullptr);

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
		assert(m_Memory != nullptr);
		memset(m_Memory, 0, m_nSize);
	}

	//------------------------------------------------------------------------------------------------------------------------------

	void CPUMemory::TakeSnapshot()
	{
		assert(m_Memory != nullptr);
		assert(m_MemorySnapshot == nullptr);
		assert(m_IsLocked);

		m_MemorySnapshot = new unsigned char[m_nSize];
		memcpy(m_MemorySnapshot, m_Memory, m_nSize);
	}

	void CPUMemory::RestoreFromSnapshot()
	{
		assert(m_Memory != nullptr);
		assert(m_MemorySnapshot != nullptr);
		assert(m_IsLocked);

		memcpy(m_Memory, m_MemorySnapshot, m_nSize);
	}

	void CPUMemory::FlushSnapshot()
	{
		assert(m_MemorySnapshot != nullptr);
		assert(m_IsLocked);

		delete[] m_MemorySnapshot;
		m_MemorySnapshot = nullptr;
	}

	//------------------------------------------------------------------------------------------------------------------------------

	unsigned char CPUMemory::GetByte(unsigned int nAddress) const
	{
		assert(m_Memory != nullptr);
		assert(nAddress < m_nSize);
		assert(m_IsLocked);

		return m_Memory[nAddress];
	}

	unsigned short CPUMemory::GetWord(unsigned int nAddress) const
	{
		assert(m_Memory != nullptr);
		assert(nAddress < m_nSize - 1);
		assert(m_IsLocked);

		return (unsigned short)m_Memory[nAddress] | (((unsigned short)m_Memory[nAddress + 1]) << 8);
	}

	void CPUMemory::GetData(unsigned int inAddress, void* inDestinationBuffer, unsigned int inDestinationBufferByteCount) const
	{
		assert(inDestinationBuffer != nullptr);
		assert(m_Memory != nullptr);
		assert(inAddress < m_nSize + inDestinationBufferByteCount);
		assert(m_IsLocked);

		unsigned char* pDest = (unsigned char*)inDestinationBuffer;

		for (unsigned int i = 0; i < inDestinationBufferByteCount; i++)
			pDest[i] = m_Memory[inAddress + i];
	}

	//------------------------------------------------------------------------------------------------------------------------------

	void CPUMemory::SetByte(unsigned int nAddress, unsigned char ucByte)
	{
		assert(m_Memory != nullptr);
		assert(nAddress < m_nSize);
		assert(m_IsLocked);

		m_Memory[nAddress] = ucByte;
	}

	void CPUMemory::SetWord(unsigned int nAddress, unsigned short usWord)
	{
		assert(m_Memory != nullptr);
		assert(nAddress < m_nSize - 1);
		assert(m_IsLocked);

		m_Memory[nAddress] = (unsigned char)(usWord & 0x00ff);
		m_Memory[nAddress + 1] = (unsigned char)((usWord & 0xff00) >> 8);
	}

	void CPUMemory::SetData(unsigned int nAddress, const void* pSourceBuffer, unsigned int nSourceBufferByteCount)
	{
		assert(pSourceBuffer != nullptr);
		assert(m_Memory != nullptr);
		assert(nAddress < m_nSize + nSourceBufferByteCount);
		assert(m_IsLocked);

		unsigned char* pSrc = (unsigned char*)pSourceBuffer;

		for (unsigned int i = 0; i < nSourceBufferByteCount; i++)
			m_Memory[nAddress + i] = pSrc[i];
	}
}