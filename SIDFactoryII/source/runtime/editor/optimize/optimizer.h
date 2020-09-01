#pragma once

#include <memory>
#include <vector>

namespace Emulation
{
	class CPUMemory;
}

namespace Editor
{
	class DriverInfo;
	class DataSourceOrderList;
	class DataSourceSequence;
	class DataSourceTable;

	class Optimizer
	{
	private:
		struct RelocationInfo
		{
			unsigned char m_From;
			unsigned char m_To;
		};

	public:
		Optimizer
		(
			Emulation::CPUMemory* inCPUMemory,
			DriverInfo& inDriverInfo,
			std::vector<std::shared_ptr<DataSourceOrderList>> inOrderListDataSources,
			std::vector<std::shared_ptr<DataSourceSequence>> inSequenceDataSources,
			std::shared_ptr<DataSourceTable> inInstrumentTableDataSource,
			std::shared_ptr<DataSourceTable> inCommandTableDataSource,
			int inInstrumentsTableID,
			int inCommandsTableID
		);

		~Optimizer();

		const std::vector<unsigned char>& GetUsedSequenceIndices() const;
		const std::vector<unsigned char>& GetUsedInstrumentIndices() const;
		const std::vector<unsigned char>& GetUsedCommandIndices() const;

		void Execute();

	private:
		void GatherOptimizationData();
		void BuildRelocationData();
		void RelocateData();

		void BuildRelocationData(const std::vector<unsigned char>& inUsedIndicesSorted, std::vector<RelocationInfo>& outRelocationData);
		void RelocateTableRows(int inTableID, std::shared_ptr<DataSourceTable>& inTableData, const std::vector<RelocationInfo>& inRelocationInfo, const std::vector<unsigned char>& inUsedIndicesSorted);
		void RelocateSequences();
		void AdjustOrderlists();
		void AdjustSequences();

		Emulation::CPUMemory* m_CPUMemory;

		// Driver info
		DriverInfo& m_DriverInfo;

		// Table IDs
		int m_InstrumentsTableID;
		int m_CommandsTableID;

		// Data sources
		std::vector<std::shared_ptr<DataSourceOrderList>> m_OrderListDataSources;
		std::vector<std::shared_ptr<DataSourceSequence>> m_SequenceDataSources;
		std::shared_ptr<DataSourceTable> m_InstrumentTableDataSource;
		std::shared_ptr<DataSourceTable> m_CommandTableDataSource;

		// Optimization data
		std::vector<unsigned char> m_UsedSequenceIndices;
		std::vector<unsigned char> m_UsedInstrumentIndices;
		std::vector<unsigned char> m_UsedCommandIndices;

		// Relocation data
		std::vector<RelocationInfo> m_SequenceRelocationData;
		std::vector<RelocationInfo> m_InstrumentRelocationData;
		std::vector<RelocationInfo> m_CommandRelocatonInfo;
	};
}