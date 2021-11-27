#include "datasource_orderlist.h"
#include "runtime/emulation/cpumemory.h"
#include <cstring>
#include "foundation/base/assert.h"

namespace Editor
{
	DataSourceOrderList::DataSourceOrderList(Emulation::CPUMemory* inCPUMemory, unsigned short inSourceAddress, int inBlockSize)
		: DataSourceEmulationMemory(inCPUMemory, inSourceAddress, inBlockSize)
		, m_Length(0)
		, m_PackedSize(0)
	{
		m_Events = new Entry[MaxEntryCount];
		m_InternalBuffer = new unsigned char[MaxEntryCount * 2 + 1];

		ClearEntries();

		PullDataFromSource();
	}

	DataSourceOrderList::~DataSourceOrderList()
	{
		delete[] m_Events;
		delete[] m_InternalBuffer;
	}


	DataSourceOrderList::Entry& DataSourceOrderList::operator[](int inIndex)
	{
		FOUNDATION_ASSERT(inIndex < MaxEntryCount);
		FOUNDATION_ASSERT(m_Events != nullptr);

		return m_Events[inIndex];
	}


	const DataSourceOrderList::Entry& DataSourceOrderList::operator[](int inIndex) const
	{
		FOUNDATION_ASSERT(inIndex < MaxEntryCount);
		FOUNDATION_ASSERT(m_Events != nullptr);

		return m_Events[inIndex];
	}


	bool DataSourceOrderList::PushDataToSource()
	{
		FOUNDATION_ASSERT(m_CPUMemory != nullptr);
		FOUNDATION_ASSERT(m_CPUMemory->IsLocked());
		FOUNDATION_ASSERT(m_Data != nullptr);

		m_CPUMemory->SetData(m_SourceAddress, m_Data, m_DataSize);

		return true;
	}


	unsigned int DataSourceOrderList::GetPackedSize() const
	{
		return m_PackedSize;
	}


	unsigned int DataSourceOrderList::GetLength() const
	{
		return m_Length;
	}


	bool DataSourceOrderList::CanIncreaseSize() const
	{
		return m_Length < static_cast<unsigned int>(MaxEntryCount);

	}

	void DataSourceOrderList::IncreaseSize()
	{
		FOUNDATION_ASSERT(CanIncreaseSize());

		++m_Length;
	}

	void DataSourceOrderList::DecreaseSize()
	{
		FOUNDATION_ASSERT(m_Length > 2);

		--m_Length;
	}


	void DataSourceOrderList::ComputeLength()
	{
		for (int i = 0; i < MaxEntryCount; ++i)
		{
			if (m_Events[i].m_Transposition >= 0xfe)
			{
				m_Length = i + 1;
				return;
			}
		}

		m_Length = MaxEntryCount - 1;
	}


	unsigned char DataSourceOrderList::GetLoopIndex() const
	{
		if (m_Length > 0)
			return m_Events[m_Length - 1].m_SequenceIndex;

		return 0;
	}


	void DataSourceOrderList::SetLoopIndex(unsigned char inLoopIndex)
	{
		if (m_Length > 0)
			m_Events[m_Length - 1].m_SequenceIndex = inLoopIndex;
	}


	DataSourceOrderList::PackResult DataSourceOrderList::Pack() const
	{
		int end_index = -1;
		int end_marker = 0;
		int loop_index = -1;

		// Find end marker and loop index
		for (int i = 0; i < MaxEntryCount; ++i)
		{
			const unsigned char val = m_Events[i].m_Transposition;
			if (val == 0xff || val == 0xfe)
			{
				end_index = i;
				end_marker = val;
				loop_index = m_Events[i].m_SequenceIndex;

				break;
			}
		}

		// If end marker wasn't found, return with no content
		if (end_index == -1)
		{
			// Orderlist was without end marker
			return PackResult(nullptr, 0);
		}

		unsigned char current_transposition = 0;
		int index = 0;
		int packed_loop_index = 0;

		// Pack order list
		for (int i = 0; i < end_index; ++i)
		{
			unsigned char transposition = m_Events[i].m_Transposition;
			unsigned char sequence_index = m_Events[i].m_SequenceIndex;

			// If transposition changed, or a loop index, add the transposition value to the order list
			if (i == loop_index || transposition != current_transposition)
			{
				if (i == loop_index)
					packed_loop_index = index;

				m_InternalBuffer[index++] = transposition;
				current_transposition = transposition;
			}

			m_InternalBuffer[index++] = sequence_index;
		}

		// Append the end marker
		FOUNDATION_ASSERT(end_marker >= 0 && end_marker < 0x100);
		m_InternalBuffer[index++] = static_cast<unsigned char>(end_marker);

		// .. and the loop point if the end marker is a loop mark
		if (end_marker == 0xff)
		{
			FOUNDATION_ASSERT(packed_loop_index >= 0 && packed_loop_index < 0x100);
			m_InternalBuffer[index++] = static_cast<unsigned char>(packed_loop_index);
		}

		// If packed data were produced, which is within the size limitations...
		if (index > 0 && index < 0x100)
		{
			// Create a buffer for the data, copy and return
			unsigned char* packed_data = new unsigned char[index];

			for (int i = 0; i < index; ++i)
				packed_data[i] = m_InternalBuffer[i];

			return PackResult(packed_data, index);
		}

		// .. otherwise return nothing
		return PackResult(nullptr, 0);
	}


	void DataSourceOrderList::SendPackedDataToBuffer(const PackResult& inPackResult)
	{
		FOUNDATION_ASSERT(inPackResult.m_DataLength <= m_DataSize);

		memset(m_Data, 0, m_DataSize);
		memcpy(m_Data, &*inPackResult.m_Data, inPackResult.m_DataLength);

		m_PackedSize = inPackResult.m_DataLength;
	}


	DataSourceOrderList::PackedDataEventPosition DataSourceOrderList::GetIndexInPackedData(int inIndex) const
	{
		int count = 0;
		unsigned char transpose = 0;
		unsigned char sequence = 0;

		for (int i = 0; i < static_cast<int>(m_PackedSize); ++i)
		{
			if (m_Data[i] < 0x80)
			{
				sequence = m_Data[i];

				++count;
				if (count == inIndex + 1)
				{
					const unsigned char next_event = m_Data[i + 1];
					const int return_index = next_event < 0xfe ? i + 1 : m_Data[i + 2];

					// The event structure is set to invalid, if the next comming event is end of sequence. This might not actually work correctly!
					return { next_event != 0xfe, static_cast<unsigned char>(return_index), sequence, transpose };
				}
			}
			else
			{
				int transpose_value = static_cast<int>((m_Data[i] & 0x7f) - 0x20);
				transpose = static_cast<unsigned int>(transpose_value & 0xff);
			}
		}

		return { false, 0, 0, 0 };
	}


	void DataSourceOrderList::PullDataFromSource()
	{
		FOUNDATION_ASSERT(m_CPUMemory != nullptr);
		FOUNDATION_ASSERT(m_Data != nullptr);

		m_CPUMemory->Lock();
		m_CPUMemory->GetData(m_SourceAddress, m_Data, m_DataSize);
		m_CPUMemory->Unlock();

		ClearEntries();
		Unpack();
	}

	void DataSourceOrderList::ClearEntries()
	{
		for (int i = 0; i < MaxEntryCount; ++i)
		{
			m_Events->m_Transposition = 0xa0;
			m_Events->m_SequenceIndex = 0;
		}
	}

	void DataSourceOrderList::Unpack()
	{
		int end_marker_index = -1;
		int entry_count = 0;
		unsigned char end_marker = 0;
		unsigned char loop_index = 0;

		// Find end and possibly loop index
		for (int i = 0; i < 0x100; ++i)
		{
			unsigned char value = m_Data[i];

			if (value == 0xff)
			{
				end_marker_index = i;
				end_marker = value;
				loop_index = m_Data[i + 1];

				++entry_count;

				m_PackedSize = i + 2;

				break;
			}
			if (value == 0xfe)
			{
				end_marker_index = i;
				end_marker = value;

				++entry_count;

				m_PackedSize = i + 1;

				break;
			}
			if (value < 0x80)
				++entry_count;
		}

		if (end_marker_index == -1 || entry_count >= MaxEntryCount)
		{
			m_Length = 0;
			return;
		}

		// Set order list length
		m_Length = entry_count;

		// Unpack from data source
		unsigned char current_transposition = 0;
		int unpacked_loop_index = 0;
		int index = 0;

		for (int i = 0; i < end_marker_index; ++i)
		{
			// Get the packed value
			unsigned char value = m_Data[i];

			// If the current index of the packed data is the loop point, store the index of the unpacked data
			if (i == loop_index)
				unpacked_loop_index = index;

			// If the value is a transpotion value, update the current transposition value
			if (value >= 0x80)
				current_transposition = value;
			else
			{
				//... otherwise, store the entry
				m_Events[index].m_Transposition = current_transposition;
				m_Events[index].m_SequenceIndex = value;

				++index;
			}
		}

		// Append endmarker, and loop index
		FOUNDATION_ASSERT(unpacked_loop_index >= 0 && unpacked_loop_index < 0x100);

		m_Events[index].m_Transposition = end_marker;
		m_Events[index].m_SequenceIndex = static_cast<unsigned char>(unpacked_loop_index);
	}
}
