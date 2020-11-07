#include "bit_array.h"
#include "foundation/base/assert.h"

namespace Utility
{
	BitArray::BitArray()
		: m_Size(0)
		, m_BufferSize(0)
		, m_Buffer(nullptr)
	{

	}

	BitArray::BitArray(unsigned int inSize)
		: m_Size(inSize)
		, m_BufferSize((inSize + 31) >> 5)
	{
		m_Buffer = new unsigned int[m_BufferSize];
	}

	BitArray::~BitArray()
	{
		delete[] m_Buffer;
	}


	bool BitArray::operator[](unsigned int inBit) const
	{
		return IsSet(inBit);
	}


	void BitArray::Resize(unsigned int inSize)
	{
		unsigned int new_buffer_size = (inSize + 31) >> 5;
		unsigned int* new_buffer = new unsigned int[new_buffer_size];

		if (m_Buffer != nullptr)
		{
			for (unsigned int i = 0; i < new_buffer_size && i < m_BufferSize; ++i)
				new_buffer[i] = m_Buffer[i];

			delete[] m_Buffer;
		}

		m_Size = inSize;
		m_Buffer = new_buffer;
		m_BufferSize = new_buffer_size;
	}


	void BitArray::Clear()
	{
		for (unsigned int i = 0; i < m_BufferSize; ++i)
			m_Buffer[i] = 0;
	}


	bool BitArray::IsSet(unsigned int inBit) const
	{
		FOUNDATION_ASSERT(inBit < m_Size);

		unsigned int buffer_index = inBit >> 5;
		unsigned int buffer_slot_mask = 1 << (inBit & 0x1f);

		return (m_Buffer[buffer_index] & buffer_slot_mask) != 0;
	}

	void BitArray::Set(unsigned int inBit)
	{
		FOUNDATION_ASSERT(inBit < m_Size);

		unsigned int buffer_index = inBit >> 5;
		unsigned int buffer_slot_mask = 1 << (inBit & 0x1f);

		m_Buffer[buffer_index] |= buffer_slot_mask;
	}

	void BitArray::Clear(unsigned int inBit)
	{
		FOUNDATION_ASSERT(inBit < m_Size);

		unsigned int buffer_index = inBit >> 5;
		unsigned int buffer_slot_mask = 1 << (inBit & 0x1f);

		m_Buffer[buffer_index] &= ~buffer_slot_mask;
	}
}