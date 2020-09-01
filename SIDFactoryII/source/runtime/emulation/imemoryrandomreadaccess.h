#pragma once

#include <vector>

namespace Emulation
{
	class IMemoryRandomReadAccess
	{
	protected:
		IMemoryRandomReadAccess() { }

	public:
		virtual const unsigned char& operator[](int i) const = 0;

		virtual unsigned char GetByte(unsigned int nAddress) const = 0;
		virtual unsigned short GetWord(unsigned int nAddress) const = 0;
		virtual void GetData(unsigned int nAddress, void* pDestinationBuffer, unsigned int nDestinationBufferByteCount) const = 0;
	};
}
