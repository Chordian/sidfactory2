#pragma once

#include "runtime/editor/driver/driver_info.h"
#include "runtime/emulation/cpumemory.h"
#include <memory>
#include <vector>

namespace Emulation
{
	class CPUMemory;
}

namespace Utility
{
	class C64File;
}

namespace Editor
{
	class Packer
	{
		struct DataSection
		{
			unsigned int m_ID;

			unsigned short m_SourceAddress;
			unsigned short m_SourceSize;
			unsigned short m_DestinationAddress;
		};

	public:
		struct TargetMemorySection
		{
			unsigned short m_Begin;
			unsigned short m_End;
		};

		Packer(Emulation::CPUMemory& inCPUMemory, const DriverInfo& inDriverInfo, unsigned short inDestinationAddress, unsigned char inLowestZP);
		~Packer();

		bool Run();
		std::string GetError();

		unsigned int GetResultCount() const;
		std::shared_ptr<Utility::C64File> GetResult(unsigned int inSection) const;

	private:

		const unsigned int AddDataSection(unsigned short inAddress, unsigned short inSize);
		const DataSection* GetDataSection(int inID) const;

		void FetchTables();
		void FetchOrderListPointers();
		void FetchSequencePointers();
		void FetchOrderLists();
		void FetchSequences();

		unsigned short ComputeDestinationAddresses();

		std::shared_ptr<Utility::C64File> CreateOutputDataContainer(unsigned short inTopAddress, unsigned short inEndAddress);
		unsigned short CopyDataToOutputContainer();

		void AdjustOrderListPointers();
		void AdjustSequencePointers();

		unsigned short GetMultiSongPatchSize();
		void ApplyMultiSongPatch(unsigned short inTargetAddress);

		unsigned short GetRelocatedVector(unsigned short inVectorAddress) const;
		void ProcessDriverCode();

		unsigned short m_DestinationAddress;
		unsigned short m_DestinationAddressDelta;

		std::vector<TargetMemorySection> m_TargetMemorySections;

		unsigned char m_CurrentLowestZP;
		unsigned char m_LowestZP;

		std::vector<DataSection> m_DataSectionList;

		unsigned char m_HighestUsedSequenceIndex;

		int m_OrderListPointersDataSectionLowID;
		int m_OrderListPointersDataSectionHighID;
		int m_SequencePointersDataSectionLowID;
		int m_SequencePointersDataSectionHighID;

		std::vector<int> m_OrderListDataSectionIDList;
		std::vector<int> m_SequenceDataSectionIDList;

		const DriverInfo& m_DriverInfo;
		Emulation::CPUMemory& m_CPUMemory;

		std::vector<unsigned short> m_OrderListAdressList;

		std::shared_ptr<Utility::C64File> m_OutputData;
	};
}