#include "runtime/editor/optimize/optimizer.h"
#include "runtime/editor/datasources/datasource_orderlist.h"
#include "runtime/editor/datasources/datasource_sequence.h"
#include "runtime/editor/datasources/datasource_table.h"
#include "runtime/editor/driver/driver_info.h"
#include "runtime/editor/auxilarydata/auxilary_data_collection.h"
#include "runtime/editor/auxilarydata/auxilary_data_table_text.h"

#include "runtime/emulation/cpumemory.h"

#include <algorithm>
#include "foundation/base/assert.h"

namespace Editor
{
	Optimizer::Optimizer
	(
		Emulation::CPUMemory* inCPUMemory,
		DriverInfo& inDriverInfo,
		std::vector<std::shared_ptr<DataSourceOrderList>> inOrderListDataSources,
		std::vector<std::shared_ptr<DataSourceSequence>> inSequenceDataSources,
		std::shared_ptr<DataSourceTable> inInstrumentTableDataSource,
		std::shared_ptr<DataSourceTable> inCommandTableDataSource,
		int inInstrumentsTableID,
		int inCommandsTableID
	)
		: m_CPUMemory(inCPUMemory)
		, m_DriverInfo(inDriverInfo)
		, m_OrderListDataSources(inOrderListDataSources)
		, m_SequenceDataSources(inSequenceDataSources)
		, m_InstrumentTableDataSource(inInstrumentTableDataSource)
		, m_CommandTableDataSource(inCommandTableDataSource)
		, m_InstrumentsTableID(inInstrumentsTableID)
		, m_CommandsTableID(inCommandsTableID)
	{
		GatherOptimizationData();
		BuildRelocationData();
	}


	Optimizer::~Optimizer() = default;


	const std::vector<unsigned char>& Optimizer::GetUsedSequenceIndices() const
	{
		return m_UsedSequenceIndices;
	}


	const std::vector<unsigned char>& Optimizer::GetUsedInstrumentIndices() const
	{
		return m_UsedInstrumentIndices;
	}


	const std::vector<unsigned char>& Optimizer::GetUsedCommandIndices() const
	{
		return m_UsedCommandIndices;
	}
	

	void Optimizer::Execute()
	{
		m_CPUMemory->Lock();
		RelocateData();
		m_CPUMemory->Unlock();
	}


	void Optimizer::GatherOptimizationData()
	{
		// Sequences referenced from orderlist
		for (const auto& orderlist : m_OrderListDataSources)
		{
			const unsigned int length = orderlist->GetLength();

			for (unsigned int i = 0; i < length; ++i)
			{
				const unsigned char transpose = (*orderlist)[i].m_Transposition;

				if (transpose < 0xfe)
				{
					const unsigned char sequence_index = (*orderlist)[i].m_SequenceIndex;

					if (std::find(m_UsedSequenceIndices.begin(), m_UsedSequenceIndices.end(), sequence_index) == m_UsedSequenceIndices.end())
						m_UsedSequenceIndices.push_back(sequence_index);
				}
			}
		}

		std::sort(m_UsedSequenceIndices.begin(), m_UsedSequenceIndices.end(), [](const auto& inA, const auto& inB) { return inA < inB; });

		for (const unsigned char& sequence_index : m_UsedSequenceIndices)
		{
			const auto& sequence = m_SequenceDataSources[sequence_index];
			const unsigned int length = sequence->GetLength();

			for (unsigned int i = 0; i < length; ++i)
			{
				const auto& event = (*sequence)[i];
				if (event.m_Instrument >= 0xa0)
				{
					const char instrument = event.m_Instrument & 0x1f;

					if (std::find(m_UsedInstrumentIndices.begin(), m_UsedInstrumentIndices.end(), instrument) == m_UsedInstrumentIndices.end())
						m_UsedInstrumentIndices.push_back(instrument);
				}
				if (event.m_Command >= 0xc0)
				{
					const char command = event.m_Command & 0x3f;

					if (std::find(m_UsedCommandIndices.begin(), m_UsedCommandIndices.end(), command) == m_UsedCommandIndices.end())
						m_UsedCommandIndices.push_back(command);
				}
			}
		}

		std::sort(m_UsedInstrumentIndices.begin(), m_UsedInstrumentIndices.end(), [](const auto& inA, const auto& inB) { return inA < inB; });
		std::sort(m_UsedCommandIndices.begin(), m_UsedCommandIndices.end(), [](const auto& inA, const auto& inB) { return inA < inB; });
	}


	void Optimizer::BuildRelocationData()
	{
		BuildRelocationData(m_UsedInstrumentIndices, m_InstrumentRelocationData);
		BuildRelocationData(m_UsedCommandIndices, m_CommandRelocatonInfo);
		BuildRelocationData(m_UsedSequenceIndices, m_SequenceRelocationData);
	}


	void Optimizer::RelocateData()
	{
		RelocateTableRows(m_InstrumentsTableID, m_InstrumentTableDataSource, m_InstrumentRelocationData, m_UsedInstrumentIndices);
		RelocateTableRows(m_CommandsTableID, m_CommandTableDataSource, m_CommandRelocatonInfo, m_UsedCommandIndices);
		RelocateSequences();
		AdjustOrderlists();
		AdjustSequences();
	}


	void Optimizer::RelocateTableRows(int inTableID, std::shared_ptr<DataSourceTable>& inTableData, const std::vector<RelocationInfo>& inRelocationInfo, const std::vector<unsigned char>& inUsedIndicesSorted)
	{
		const int stride = inTableData->GetColumnCount();

		if (inRelocationInfo.size() > 0)
		{
			int highest_to = 0;

			// Relocate table entries
			for (const RelocationInfo& relocation_info : inRelocationInfo)
			{
				const int index_from = stride * relocation_info.m_From;
				const int index_to = stride * relocation_info.m_To;

				if (relocation_info.m_To > highest_to)
					highest_to = relocation_info.m_To;

				for (int i = 0; i < stride; ++i)
					(*inTableData)[index_to + i] = (*inTableData)[index_from + i];
			}

			// Clear the rest of the table
			for (int i = (highest_to + 1) * stride; i < inTableData->GetSize(); ++i)
				(*inTableData)[i] = 0;

			inTableData->PushDataToSource();

			// Move description text, if table has any
			auto& text_data = m_DriverInfo.GetAuxilaryDataCollection().GetTableText();

			if (text_data.HasText(inTableID))
			{
				for (const RelocationInfo& relocation_info : inRelocationInfo)
				{
					std::string desc = text_data.GetText(inTableID, relocation_info.m_From);
					text_data.SetText(inTableID, relocation_info.m_To, desc);
				}

				for (int i = highest_to + 1; i < inTableData->GetSize() / stride; ++i)
					text_data.SetText(inTableID, i, "");
			}
		}
		else
		{
			int first_clear_index = !inUsedIndicesSorted.empty() ? static_cast<int>(inUsedIndicesSorted.back() + 1) : 0;

			// Clear the rest of the table
			for (int i = first_clear_index * stride; i < inTableData->GetSize(); ++i)
				(*inTableData)[i] = 0;

			inTableData->PushDataToSource();

			// Move description text, if table has any
			auto& text_data = m_DriverInfo.GetAuxilaryDataCollection().GetTableText();

			if (text_data.HasText(inTableID))
			{
				for (int i = first_clear_index; i < inTableData->GetSize() / stride; ++i)
					text_data.SetText(inTableID, i, "");
			}
		}
	}


	void Optimizer::RelocateSequences()
	{
		if (m_SequenceRelocationData.size() > 0)
		{
			int highest_to = 0;

			for (const RelocationInfo& relocation_info : m_SequenceRelocationData)
			{
				const int index_from = relocation_info.m_From;
				const int index_to = relocation_info.m_To;

				if (relocation_info.m_To > highest_to)
					highest_to = relocation_info.m_To;

				(*m_SequenceDataSources[index_to]) = (*m_SequenceDataSources[index_from]);
			}

			for (int i = highest_to + 1; i < static_cast<int>(m_SequenceDataSources.size()); ++i)
			{
				m_SequenceDataSources[i]->ClearEvents();
				m_SequenceDataSources[i]->SetLength(1);
			}
		}
	}


	void Optimizer::AdjustOrderlists()
	{
		if (m_SequenceRelocationData.size() > 0)
		{
			// Sequences referenced from orderlist
			for (const auto& orderlist : m_OrderListDataSources)
			{
				const unsigned int length = orderlist->GetLength();

				for (unsigned int i = 0; i < length; ++i)
				{
					const unsigned char transpose = (*orderlist)[i].m_Transposition;

					if (transpose < 0xfe)
					{
						unsigned char sequence_index = (*orderlist)[i].m_SequenceIndex;
						const auto it = std::find_if(m_SequenceRelocationData.begin(), m_SequenceRelocationData.end(), [&](const auto& data) { return data.m_From == sequence_index; });
						if (it != m_SequenceRelocationData.end())
							(*orderlist)[i].m_SequenceIndex = (*it).m_To;
					}
				}

				DataSourceOrderList::PackResult packed_result = orderlist->Pack();

				if (packed_result.m_DataLength < 0x100)
					orderlist->SendPackedDataToBuffer(packed_result);

				orderlist->PushDataToSource();
			}
		}
	}


	void Optimizer::AdjustSequences()
	{
		for (const auto& sequence : m_SequenceDataSources)
		{
			const unsigned int length = sequence->GetLength();

			for (unsigned int i = 0; i < length; ++i)
			{
				auto& event = (*sequence)[i];
				if (event.m_Instrument >= 0xa0 && m_InstrumentRelocationData.size() > 0)
				{
					const char instrument = event.m_Instrument & 0x1f;

					const auto it = std::find_if(m_InstrumentRelocationData.begin(), m_InstrumentRelocationData.end(), [&](const auto& data) { return data.m_From == instrument; });
					if(it != m_InstrumentRelocationData.end())
						event.m_Instrument = ((*it).m_To | 0xa0);
				}
				if (event.m_Command >= 0xc0 && m_CommandRelocatonInfo.size() > 0)
				{
					const char command = event.m_Command & 0x3f;

					const auto it = std::find_if(m_CommandRelocatonInfo.begin(), m_CommandRelocatonInfo.end(), [&](const auto& data) { return data.m_From == command; });
					if(it != m_CommandRelocatonInfo.end())
						event.m_Command = ((*it).m_To | 0xc0);
				}
			}

			DataSourceSequence::PackResult packed_result = sequence->Pack();

			if (packed_result.m_DataLength < 0x100 && packed_result.m_Data != nullptr)
				sequence->SendPackedDataToBuffer(packed_result);
				
			sequence->PushDataToSource();
		}
	}


	void Optimizer::BuildRelocationData(const std::vector<unsigned char>& inUsedIndicesSorted, std::vector<Optimizer::RelocationInfo>& outRelocationData)
	{
		unsigned char target_index = 0;

		for (unsigned char source_index : inUsedIndicesSorted)
		{
			if (target_index != source_index)
				outRelocationData.push_back({ source_index, target_index });

			++target_index;
		}
	}
}

