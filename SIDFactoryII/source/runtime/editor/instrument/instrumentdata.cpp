#include "runtime/editor/instrument/instrumentdata.h"
#include "runtime/editor/instrument/instrumentdata_table.h"
#include "runtime/editor/instrument/instrumentdata_tablemapping.h"
#include "runtime/editor/driver/driver_info.h"
#include "runtime/editor/driver/driver_utils.h"
#include "runtime/editor/datasources/datasource_table.h"
#include "runtime/editor/components/component_base.h"
#include "runtime/editor/components/component_table_row_elements.h"
#include "runtime/editor/components_manager.h"
#include "runtime/emulation/cpumemory.h"
#include <assert.h>

namespace Editor
{
	namespace Details
	{
		std::shared_ptr<DataSourceTable> GetDataSourceTable(unsigned char inTableID, const Editor::DriverInfo& inDriverInfo, Emulation::CPUMemory& inCPUMemory)
		{
			const auto& table_definitions = inDriverInfo.GetTableDefinitions();

			for (const auto& table_definition : table_definitions)
			{
				if (table_definition.m_ID == inTableID)
					return DriverUtils::CreateTableDataSource(table_definition, &inCPUMemory);
			}

			return std::shared_ptr<DataSourceTable>();
		}

		std::shared_ptr<InstrumentDataTable> CreateInstrumentDataTable(unsigned char inStartIndex, const DataSourceTable* inTable, const DriverInfo::InstrumentDataPointerDescription& inTablePointerDescription)
		{
			return std::shared_ptr<InstrumentDataTable>();
		}

		unsigned char GetTableID(DriverInfo::TableType inTableType, const Editor::DriverInfo& inDriverInfo)
		{
			assert(inTableType != DriverInfo::TableType::Generic);

			const auto& table_definitions = inDriverInfo.GetTableDefinitions();

			for (const auto& table_definition : table_definitions)
			{
				if (table_definition.m_Type == inTableType)
					return table_definition.m_ID;
			}

			return 0xff;
		}
	}

	std::shared_ptr<InstrumentData> InstrumentData::Create(int inInstrumentIndex, const DriverInfo& inDriverInfo, const ComponentsManager& inComponentManager)
	{
		// Create the data container
		std::shared_ptr<InstrumentData> instrument_data = std::shared_ptr<InstrumentData>(new InstrumentData());

		// Get the instrument table from the cpu memory
		int instruments_table_id = static_cast<int>(Details::GetTableID(DriverInfo::TableType::Instruments, inDriverInfo));
		const ComponentTableRowElements* component_instruments = static_cast<const ComponentTableRowElements*>(inComponentManager.GetComponent(instruments_table_id));
		assert(component_instruments != nullptr);
		const DataSourceTable* instruments_table = component_instruments->GetDataSource();
		assert(instruments_table != nullptr);

		// Push the instrument values to the instrument data array
		for (unsigned int i = 0; i < instruments_table->GetColumnCount(); ++i)
		{
			unsigned char value = (*instruments_table)[i];
			instrument_data->m_InstrumentData.push_back(value);
		}

		// Run through table pointers and create instrument_tables for each
		const auto& data_description = inDriverInfo.GetInstrumentDataDescription();
		for (const auto& table_pointer_description : data_description.m_InstrumentDataPointerDescriptions)
		{
			unsigned char conditional_value = instrument_data->m_InstrumentData[table_pointer_description.m_InstrumentDataConditionalValuePosition];
			if ((conditional_value & table_pointer_description.m_ConditionValueAndValue) == table_pointer_description.m_ConditionEqualityValue)
			{
				const ComponentTableRowElements* component = static_cast<const ComponentTableRowElements*>(inComponentManager.GetComponent(table_pointer_description.m_TableID));
				assert(component != nullptr);

				const DataSourceTable* table = component->GetDataSource();
				assert(table != nullptr);

				unsigned char start_index = instrument_data->m_InstrumentData[table_pointer_description.m_InstrumentDataPointerPosition];
				start_index &= table_pointer_description.m_PointerAndValue;

				InstrumentDataTableMapping tableMapping(table, table_pointer_description);
				tableMapping.BuildFrom(start_index);

				std::shared_ptr<InstrumentDataTable> instrument_data_table = Details::CreateInstrumentDataTable(start_index, table, table_pointer_description);
				//assert(instrument_data_table != nullptr);
				if(instrument_data_table != nullptr)
					instrument_data->m_InstrumentTableData.push_back(instrument_data_table);
			}
		}

		return instrument_data;
	}


	std::shared_ptr<InstrumentData> InstrumentData::Create(const void* inData, unsigned int inDataSize)
	{
		std::shared_ptr<InstrumentData> instrument_data = std::shared_ptr<InstrumentData>(new InstrumentData());
		return instrument_data;
	}


	bool InstrumentData::IsInstrumentData(const void* inData, unsigned int inDataSize)
	{
		return false;
	}


	InstrumentData::InstrumentData()
	{

	}


	InstrumentData::~InstrumentData()
	{

	}


	std::vector<unsigned char> InstrumentData::GetData() const
	{
		return std::vector<unsigned char>();
	}
}