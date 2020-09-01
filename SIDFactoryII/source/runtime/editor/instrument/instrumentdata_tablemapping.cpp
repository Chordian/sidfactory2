#include "runtime/editor/instrument/instrumentdata_tablemapping.h"
#include "runtime/editor/datasources/datasource_table.h"
#include <assert.h>

namespace Editor
{
	InstrumentDataTableMapping::InstrumentDataTableMapping(const DataSourceTable* inTableData, const DriverInfo::InstrumentDataPointerDescription& inTablePointerDescription)
		: m_TableData(inTableData)
		, m_TablePointerDescription(inTablePointerDescription)
		, m_HighestIndex(-1)
	{
		assert(m_TableData != nullptr);
		
		m_Indices.clear();

		for (unsigned int i = 0; i < m_TableData->GetRowCount(); ++i)
			m_Indices.push_back(-1);
	}


	InstrumentDataTableMapping::~InstrumentDataTableMapping()
	{

	}


	bool InstrumentDataTableMapping::BuildFrom(int inIndex)
	{
		assert(inIndex < m_TableData->GetRowCount());

		int current_index = inIndex;

		if (m_TablePointerDescription.m_TableDataType == 0)
		{
			// Single entry only
			if (m_Indices[current_index] == -1)
			{
				++m_HighestIndex;
				m_Indices[current_index] = m_HighestIndex;
			}

			return true;
		}
		else
		{
			while (current_index < m_TableData->GetRowCount())
			{
				if (m_Indices[current_index] != -1)
					return true;

				++m_HighestIndex;
				m_Indices[current_index] = m_HighestIndex;

				current_index = GetNextIndex(current_index);
			}
		}

		return false;
	}


	int InstrumentDataTableMapping::GetNextIndex(int inIndex)
	{
		const int stride = m_TableData->GetColumnCount();

		const int table_index = stride * inIndex;
		const int jump_mark_index = m_TablePointerDescription.m_TableJumpMarkerValuePosition;
		const int jump_target_index = m_TablePointerDescription.m_TableJumpDestinationIndexPosition;

		if ((*m_TableData)[table_index + jump_mark_index] == m_TablePointerDescription.m_TableJumpMarkerValue)
			return (*m_TableData)[table_index + jump_target_index];

		return inIndex + 1;
	}
}
