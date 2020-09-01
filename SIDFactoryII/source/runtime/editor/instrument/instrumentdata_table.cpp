#include "runtime/editor/instrument/instrumentdata_table.h"

namespace Editor
{
	InstrumentDataTable::InstrumentDataTable(unsigned char inID, unsigned int inColumnCount, unsigned int inRowCount)
		: m_ID(inID)
		, m_ColumnCount(inColumnCount)
		, m_RowCount(inRowCount)
	{
	}

	InstrumentDataTable::~InstrumentDataTable()
	{
	}
}