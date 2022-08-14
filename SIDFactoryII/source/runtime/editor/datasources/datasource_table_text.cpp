#include "runtime/editor/datasources/datasource_table_text.h"
#include "runtime/editor/auxilarydata/auxilary_data_table_text.h"
#include "foundation/base/assert.h"

namespace Editor
{
	DataSourceTableText::DataSourceTableText(int inTableID, int inRowCount, AuxilaryDataTableText& inAuxDataTableText)
		: m_AuxDataTableText(inAuxDataTableText)
		, m_TableID(inTableID)
		, m_TableRowCount(inRowCount)
		, m_TextLayer(0)
	{
		PullDataFromSource();
	}

	DataSourceTableText::DataSourceTableText(int inTableID, int inRowCount, unsigned int inTextLayer, AuxilaryDataTableText& inAuxDataTableText)
		: m_AuxDataTableText(inAuxDataTableText)
		, m_TableID(inTableID)
		, m_TableRowCount(inRowCount)
		, m_TextLayer(inTextLayer)
	{
		PullDataFromSource();
	}

	bool DataSourceTableText::PushDataToSource()
	{
		const int size = GetSize();

		for (int i = 0; i < size; ++i)
			m_AuxDataTableText.SetText(m_TableID, i, m_List[i], m_TextLayer);

		return true;
	}

	bool DataSourceTableText::PullDataFromSource()
	{
		m_List.clear();

		for (int i = 0; i < m_TableRowCount; ++i)
			m_List.push_back(m_AuxDataTableText.GetText(m_TableID, i, m_TextLayer));

		return true;
	}
}
