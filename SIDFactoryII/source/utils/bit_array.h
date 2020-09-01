#pragma once

namespace Utility
{
	class BitArray
	{
	public:
		BitArray();
		BitArray(unsigned int inSize);
		~BitArray();

		bool operator[](unsigned int inBit) const;

		void Resize(unsigned int inSize);
		void Clear();

		void Set(unsigned int inBit);
		void Clear(unsigned int inBit);

	private:
		bool IsSet(unsigned int inBit) const;

		unsigned int m_Size;
		unsigned int m_BufferSize;

		unsigned int* m_Buffer;
	};
}