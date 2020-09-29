#pragma once

#include "runtime/editor/converters/iconverter.h"
#include "runtime/editor/driver/driver_info.h"

namespace Emulation
{
	class CPUMemory;
}

namespace Editor
{
	class DataSourceSequence;

	class ConverterJCH : public IConverter
	{
	private:
		struct JCH20g4Info
		{
			unsigned short m_FineTuneAddress;
			unsigned short m_WaveTableAddress;
			unsigned short m_FilterTableAddress;
			unsigned short m_PulseTableAddress;
			unsigned short m_InstrumentTableAddress;
			unsigned short m_CommandTableAddress;
			unsigned short m_OrderlistV1Address;
			unsigned short m_OrderlistV2Address;
			unsigned short m_OrderlistV3Address;
			unsigned short m_SequenceVectorLowAddress;
			unsigned short m_SequenceVectorHighAddress;
			unsigned short m_SpeedSettingAddress;
		};

	public:
		ConverterJCH();
		virtual ~ConverterJCH();

		virtual bool Convert
		(
			void* inData,
			unsigned int inDataSize,
			Foundation::IPlatform* inPlatform,
			ComponentsManager& inComponentsManager,
			std::function<void(std::shared_ptr<Utility::C64File>)> inSuccessAction
		) override;

	private:
		bool LoadDestinationDriver(Foundation::IPlatform* inPlatform);
		void GatherInputInfo();
		bool ImportTables();
		bool BuildTempoTable();
		bool BuildInitTable();
		unsigned int ImportOrderLists();
		void ImportSequences(unsigned int inMaxSequenceIndex);
		void ImportSequence(unsigned short inReadAddress, std::shared_ptr<DataSourceSequence>& inWriteDataSource);
		bool ReflectToOutput();
		void CopyTable(unsigned short inSourceAddress, unsigned short inDestinationAddress, unsigned short inSize);
		void CopyTableRowToColumnMajor(unsigned short inSourceAddress, unsigned short inDestinationAddress, unsigned short inRowCount, unsigned short inColumnCount);		
		void CopyWaveTable(unsigned short inSourceAddress, unsigned short inDestinationAddress, unsigned short inSize);

		bool CanConvertInput(void* inData, unsigned int inDataSize) const;


		// Input data parsed to c64 file
		std::shared_ptr<Utility::C64File> m_InputData;

		// Output data
		std::shared_ptr<Utility::C64File> m_OutputData;

		// Memory 
		Emulation::CPUMemory* m_CPUMemory;

		// Driver info for output
		std::shared_ptr<DriverInfo> m_DriverInfo;

		// Input file info
		JCH20g4Info m_InputInfo;

		// Success function callback
		std::function<void(std::shared_ptr<Utility::C64File>)> m_SuccessAction;
	};
}