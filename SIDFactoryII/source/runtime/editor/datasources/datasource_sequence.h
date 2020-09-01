#pragma once

#include "idatasource.h"
#include "datasource_emulation_memory.h"

#include <memory>

namespace Emulation
{
	class CPUMemory;
}

namespace Editor
{
	class DriverInfo;
	class DriverState;

	class DataSourceSequence : public DataSourceEmulationMemory
	{
	public:
		static const unsigned int MaxEventCount;

		struct Event
		{
			unsigned char m_Instrument;
			unsigned char m_Command;
			unsigned char m_Note;

			void operator=(const Event& inRhs)
			{
				m_Instrument = inRhs.m_Instrument;
				m_Command = inRhs.m_Command;
				m_Note = inRhs.m_Note;
			}

			void Clear()
			{
				m_Instrument = 0x80;
				m_Command = 0x80;
				m_Note = 0x00;
			}
		};

		struct PackedDataEventPosition
		{
			bool m_IsValid;
			unsigned char m_Index;
			unsigned char m_RemaningTicks;
			unsigned char m_CurrentDurationValue;
			unsigned char m_NextInstrumentAddress;
			bool m_NextIsEndOfSequence;
		};

		struct PackResult
		{
			PackResult()
				: m_Data(nullptr)
				, m_DataLength(-1)
			{
			}

			PackResult(unsigned char* inData, int inDataLength)
				: m_Data(inData)
				, m_DataLength(inDataLength)
			{
			}

			std::shared_ptr<unsigned char> m_Data;
			int m_DataLength;
		};

		DataSourceSequence(
			Emulation::CPUMemory* inCPUMemory, 
			const Editor::DriverInfo& inDriverInfo,
			const Editor::DriverState& inDriverState,
			unsigned char inSequenceIndex,
			unsigned short inSourceAddress, 
			int inBlockSize
		);
		DataSourceSequence(const DataSourceSequence& inOther);
		virtual ~DataSourceSequence();

		void operator=(const DataSourceSequence& inRhs);

		Event& operator[](int inIndex);
		const Event& operator[](int inIndex) const;
		bool PushDataToSource() override;
		void PullDataFromSource() override;

		unsigned int GetPackedSize() const;
		unsigned int GetLength() const;
		void SetLength(unsigned int inLength);

		unsigned char GetLastInstrumentSet() const;
		unsigned char GetLastCommandSet() const;

		PackResult Pack();
		void SendPackedDataToBuffer(const PackResult& inPackResult);
		bool IsInErrorState() const;

		PackedDataEventPosition GetEventPositionInPackedData(int inEventPosition) const;

		void ClearEvents();
	private:

		void Unpack();

		const Editor::DriverInfo& m_DriverInfo;
		const Editor::DriverState& m_DriverState;
		const unsigned char m_SequenceIndex;

		unsigned int m_Length;		
		Event* m_Events;

		unsigned char m_LastInstrumentSet;
		unsigned char m_LastCommandSet;

		unsigned char* m_InternalBuffer;
		unsigned int m_PackedSize;

		bool m_PackingErrorState;
	};
}
