#if !defined(__MEMORY_H__)
#define __MEMORY_H__

#include "assert.h"
#include "foundation/platform/iplatform.h"
#include "foundation/platform/imutex.h"
#include "runtime/emulation/imemoryrandomreadaccess.h"

namespace Emulation
{
	class IPlatformFactory;

	class CPUMemory : public IMemoryRandomReadAccess
	{
	public:
		CPUMemory(unsigned int inSize, Foundation::IPlatform* inPlatform);
		~CPUMemory();

		const unsigned char& operator[](int inAddress) const override
		{
			assert(inAddress >= 0);
			assert(inAddress < (int)m_nSize);
			assert(m_IsLocked);

			return m_Memory[inAddress];
		}

		unsigned char& operator[](int inAddress)
		{
			assert(inAddress >= 0);
			assert(inAddress < (int)m_nSize);
			assert(m_IsLocked);

			return m_Memory[inAddress];
		}

		void Lock();
		void Unlock();
		bool IsLocked() const;

		unsigned int GetSize() const { return m_nSize; }

		void Clear();

		void TakeSnapshot();
		void RestoreFromSnapshot();
		void FlushSnapshot();

		unsigned char GetByte(unsigned int inAddress) const override;
		unsigned short GetWord(unsigned int inAddress) const override;
		void GetData(unsigned int inAddress, void* inDestinationBuffer, unsigned int inDestinationBufferByteCount) const override;

		void SetByte(unsigned int inAddress, unsigned char inByteValue);
		void SetWord(unsigned int inAddress, unsigned short inWordValue);
		void SetData(unsigned int inAddress, const void* inSourceBuffer, unsigned int inSourceBufferByteCount);

		unsigned int GetAddress(void* inMemoryOffsetPointer) const 
		{
			unsigned int iAddress = static_cast<unsigned int>(static_cast<unsigned char*>(inMemoryOffsetPointer) - m_Memory);

			assert(iAddress < m_nSize);
			assert(m_IsLocked);

			return iAddress;
		};

	private:
		std::shared_ptr<Foundation::IMutex> m_Mutex;

		bool m_IsLocked;

		unsigned int m_nSize;
		unsigned char* m_Memory;
		unsigned char* m_MemorySnapshot;
	};
}

#endif //__MEMORY_H__