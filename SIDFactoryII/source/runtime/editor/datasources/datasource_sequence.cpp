#include "datasource_sequence.h"
#include "runtime/emulation/cpumemory.h"
#include "runtime/editor/driver/driver_info.h"
#include "runtime/editor/driver/driver_state.h"
#include <cstring>
#include <assert.h>

namespace Editor
{
    const unsigned int DataSourceSequence::MaxEventCount = 1024;

	DataSourceSequence::DataSourceSequence(
		Emulation::CPUMemory* inCPUMemory,
		const Editor::DriverInfo& inDriverInfo,
		const Editor::DriverState& inDriverState,
		const unsigned char inSequenceIndex,
		unsigned short inSourceAddress, 
		int inBlockSize
	)
		: DataSourceEmulationMemory(inCPUMemory, inSourceAddress, inBlockSize)
		, m_DriverInfo(inDriverInfo)
		, m_DriverState(inDriverState)
		, m_SequenceIndex(inSequenceIndex)
		, m_Length(0)
		, m_PackedSize(0)
		, m_PackingErrorState(false)
	{
		m_Events = new Event[MaxEventCount];
		m_InternalBuffer = new unsigned char[MaxEventCount * 3];

		ClearEvents();

		PullDataFromSource();
	}

	DataSourceSequence::DataSourceSequence(const DataSourceSequence& inOther)
		: DataSourceEmulationMemory(inOther.m_CPUMemory, inOther.m_SourceAddress, inOther.m_DataSize)
		, m_DriverInfo(inOther.m_DriverInfo)
		, m_DriverState(inOther.m_DriverState)
		, m_SequenceIndex(inOther.m_SequenceIndex)
		, m_Length(inOther.m_Length)
		, m_PackedSize(inOther.m_PackedSize)
		, m_PackingErrorState(inOther.m_PackingErrorState)
	{
		m_Events = new Event[MaxEventCount];
		m_InternalBuffer = new unsigned char[MaxEventCount * 3];

		for (int i = 0; i < MaxEventCount; ++i)
			m_Events[i] = inOther.m_Events[i];
	}

	DataSourceSequence::~DataSourceSequence()
	{
		delete[] m_Events;
		delete[] m_InternalBuffer;
	}

	//------------------------------------------------------------------------------------------------------------------
	
	void DataSourceSequence::operator=(const DataSourceSequence& inRhs)
	{
		for (int i = 0; i < MaxEventCount; ++i)
			m_Events[i] = inRhs.m_Events[i];

		m_Length = inRhs.m_Length;
	}

	DataSourceSequence::Event& DataSourceSequence::operator[](int inIndex)
	{
		assert(inIndex >= 0);
		assert(inIndex < MaxEventCount);

		return m_Events[inIndex];
	}

	const DataSourceSequence::Event& DataSourceSequence::operator[](int inIndex) const
	{
		assert(inIndex >= 0);
		assert(inIndex < MaxEventCount);

		return m_Events[inIndex];
	}

	//------------------------------------------------------------------------------------------------------------------

	bool DataSourceSequence::PushDataToSource()
	{
		assert(m_CPUMemory != nullptr);
		assert(m_CPUMemory->IsLocked());
		assert(m_Data != nullptr);

		const bool may_push_sequence_data = [&]()
		{
			const unsigned short current_sequence_index_address = m_DriverInfo.GetDriverCommon().m_CurrentSequenceAddress;
			const unsigned short current_sequence_cursor_position_address = m_DriverInfo.GetDriverCommon().m_SequenceIndexAddress;

			if (m_DriverState.GetPlayState() == DriverState::PlayState::Playing)
			{
				for (int i = 0; i < m_DriverInfo.GetMusicData().m_TrackCount; ++i)
				{
					const unsigned char current_sequence_index = m_CPUMemory->GetByte(current_sequence_index_address + i);
					const unsigned char current_sequence_cursor_position = m_CPUMemory->GetByte(current_sequence_cursor_position_address + i);

					if (current_sequence_index == m_SequenceIndex && current_sequence_cursor_position != 0)
					{
						int change_from = 256;

						for (int j = 0; j < m_DataSize; ++j)
						{
							const char new_data = m_Data[j];
							const char old_data = (*m_CPUMemory)[m_SourceAddress + j];

							if((new_data & 0xe0) == 0x80 || (old_data & 0xe0) == 0x80)
							{
								if (new_data != old_data)
								{
									change_from = j;
									break;
								}
							}
						}

						return current_sequence_cursor_position < change_from;
					}
				}
			}

			return true;
		}();

		if(may_push_sequence_data)
		{
			m_CPUMemory->SetData(m_SourceAddress, m_Data, m_DataSize);
			return true;
		}

		return false;
	}


	unsigned int DataSourceSequence::GetPackedSize() const
	{
		return m_PackedSize;
	}

	unsigned int DataSourceSequence::GetLength() const
	{
		return m_Length;
	}

	void DataSourceSequence::SetLength(unsigned int inLength)
	{
		assert(inLength <= MaxEventCount);
		m_Length = inLength;
	}


	//------------------------------------------------------------------------------------------------------------------

	void DataSourceSequence::PullDataFromSource()
	{
		assert(m_CPUMemory != nullptr);
		assert(m_Data != nullptr);

		m_CPUMemory->Lock();
		m_CPUMemory->GetData(m_SourceAddress, m_Data, m_DataSize);
		m_CPUMemory->Unlock();

		ClearEvents();
		Unpack();
	}


	void DataSourceSequence::ClearEvents()
	{
		m_LastCommandSet = 0xff;
		m_LastInstrumentSet = 0xff;

		for (int i = 0; i < MaxEventCount; ++i)
			m_Events[i].Clear();
	}

	//------------------------------------------------------------------------------------------------------------------
	// Data format
	//
	// 00 		= Note off
	// 01 - 6f 	= Notes
	// 70 - 7d 	= Reserved
	// 7e 		= Note on
	// 7f		= End of sequence
	// 80 - 8f	= Duration
	// 90 - 9f	= Duration, note is tied! (That's the one coming this tick)
	// a0 - bf	= Set instrument ($00 - $1f)
	// c0 - ff	= Set command ($00 - $3f)
	//------------------------------------------------------------------------------------------------------------------

	void DataSourceSequence::Unpack()
	{
		ClearEvents();

		int event_index = 0;
		int duration = 0;
		bool tie_note = false;

		for (int i = 0; i < 0x100;)
		{
			unsigned char value = m_Data[i++];

			if (value == 0x7f)
			{
				m_PackedSize = i;
				break;
			}

			if (value >= 0xc0)	// Command		
			{
				m_Events[event_index].m_Command = value;
				m_LastCommandSet = value & 0x3f;
				value = m_Data[i++];

				assert(i < 0x100);
			}
			else
				m_Events[event_index].m_Command = 0x80;

			if (value >= 0xa0) // Instrument
			{
				m_Events[event_index].m_Instrument = value;
				m_LastInstrumentSet = value & 0x1f;
				value = m_Data[i++];

				assert(i < 0x100);
			}
			else
				m_Events[event_index].m_Instrument = 0x80;

			if (value >= 0x80)
			{
				duration = value & 0x0f;

				tie_note = (value & 0x10) != 0;

				if (tie_note)
					m_Events[event_index].m_Instrument = 0x90;

				value = m_Data[i++];

				assert(i < 0x100);
			}

			assert(value < 0x80);

			m_Events[event_index++].m_Note = value;

			for (int j = 0; j < duration; ++j)
			{
				m_Events[event_index].m_Command = 0x80;
				m_Events[event_index].m_Instrument = 0x80;
				m_Events[event_index].m_Note = value != 0x00 ? 0x7e : 0x00;

				event_index++;
			}
		}

		m_Length = event_index;
		m_PackingErrorState = false;
	}


	unsigned char DataSourceSequence::GetLastInstrumentSet() const
	{
		return m_LastInstrumentSet;
	}


	unsigned char DataSourceSequence::GetLastCommandSet() const
	{
		return m_LastCommandSet;
	}


	DataSourceSequence::PackResult DataSourceSequence::Pack()
	{
		m_LastCommandSet = 0xff;
		m_LastInstrumentSet = 0xff;

		int packIndex = 0;
		int lastDuration = -1;

		for (unsigned int i = 0; i < m_Length; ++i)
		{
			unsigned char instrument = m_Events[i].m_Instrument;
			unsigned char command = m_Events[i].m_Command;
			unsigned char note = m_Events[i].m_Note;

			// Look for next event
			int duration = 0;

			for (unsigned int j = i + 1; j < m_Length; ++j)
			{
				if (m_Events[j].m_Instrument != 0x80 || m_Events[j].m_Command != 0x80)
					break;

				if (note == 0)
				{
					if (m_Events[j].m_Note != 0)
						break;
				}
				else
				{
					if (m_Events[j].m_Note != 0x7e)
						break;
				}

				duration++;

				if (duration >= 0x0f)
					break;
			}

			bool bTieNote = (instrument == 0x90);

			if (command != 0x80)
			{
				m_InternalBuffer[packIndex++] = command;
				m_LastCommandSet = command & 0x3f;
			}
			if (instrument >= 0xa0)
			{
				m_InternalBuffer[packIndex++] = instrument;
				m_LastInstrumentSet = instrument & 0x1f;
			}

			if (lastDuration != duration || bTieNote)
			{
				m_InternalBuffer[packIndex++] = static_cast<unsigned char>((duration | 0x80) | (bTieNote ? 0x10 : 0x00));
				lastDuration = duration;
			}

			m_InternalBuffer[packIndex++] = note;

			i += duration;
		}

		// Create a buffer of the exact size of the packed data and copy the data into it
		m_PackingErrorState = !(packIndex > 0 && packIndex < 0xff);

		if(!m_PackingErrorState)
		{
			// Insert end mark
			m_InternalBuffer[packIndex++] = 0x7f;

			unsigned char* packed_data = new unsigned char[packIndex];

			for (int i = 0; i < packIndex; i++)
				packed_data[i] = m_InternalBuffer[i];

			m_PackingErrorState = false;

			return PackResult(packed_data, packIndex);
		}

		return PackResult();
	}

	void DataSourceSequence::SendPackedDataToBuffer(const PackResult& inPackResult)
	{
		assert(inPackResult.m_DataLength <= m_DataSize);

		memset(m_Data, 0, m_DataSize);
		memcpy(m_Data, &*inPackResult.m_Data, inPackResult.m_DataLength);

		m_PackedSize = inPackResult.m_DataLength;
	}


	bool DataSourceSequence::IsInErrorState() const
	{
		return m_PackingErrorState;
	}



	DataSourceSequence::PackedDataEventPosition DataSourceSequence::GetEventPositionInPackedData(int inEventPosition) const
	{
		int event_position = 0;
		int index = 0;
		unsigned char current_delta_tick = 0;
		unsigned char next_instrument = 0;

		while (index < static_cast<int>(m_PackedSize))
		{
			if (event_position >= inEventPosition)
				return { true, static_cast<unsigned char>(index), static_cast<unsigned char>(event_position - inEventPosition), current_delta_tick, next_instrument, m_Data[index] == 0x7f };

			while (m_Data[index] >= 0x80)
			{
				if (m_Data[index] < 0xa0)
					current_delta_tick = m_Data[index] & 0x0f;
				else if (m_Data[index] < 0xc0)
					next_instrument = m_Data[index];

				++index;
			}

			event_position += (current_delta_tick + 1);
			++index;
		}

		return { false, 0, 0, 0, 0, false };
	}
}
