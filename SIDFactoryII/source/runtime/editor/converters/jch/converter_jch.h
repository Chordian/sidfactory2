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
			unsigned short m_WaveTableAddress = 0x0fbc;
			unsigned short m_FilterTableAddress = 0x0fc0;
			unsigned short m_PulseTableAddress = 0x0fc2;
			unsigned short m_InstrumentTableAddress = 0x0fc4;
			unsigned short m_CommandTableAddress = 0x0fd0;
			unsigned short m_OrderlistV1Address = 0x0fc6;
			unsigned short m_OrderlistV2Address = 0x0fc8;
			unsigned short m_OrderlistV3Address = 0x0fca;
			unsigned short m_SequenceVectorLowAddress = 0x0fcc;
			unsigned short m_SequenceVectorHighAddress = 0x0fce;
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
		unsigned int ImportOrderLists();
		void ImportSequences(unsigned int inMaxSequenceIndex);
		void ImportSequence(unsigned short inReadAddress, std::shared_ptr<DataSourceSequence>& inWriteDataSource);
		bool ReflectToOutput();
		void CopyTable(unsigned short inSourceAddress, unsigned short inDestinationAddress, unsigned short inSize);
		void CopyTableRowToColumnMajor(unsigned short inSourceAddress, unsigned short inDestinationAddress, unsigned short inRowCount, unsigned short inColumnCount);		
		void CopyWaveTable(unsigned short inSourceAddress, unsigned short inDestinationAddress, unsigned short inSize);

		bool IsFileValid(void* inData, unsigned int inDataSize) const;


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