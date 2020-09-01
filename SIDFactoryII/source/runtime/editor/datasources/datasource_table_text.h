#pragma once

#include "runtime/editor/datasources/datasource_tlist.h"
#include <vector>
#include <string>

namespace Editor
{
	class AuxilaryDataTableText;

	class DataSourceTableText : public DataSourceTList<std::string>
	{
	public:
		DataSourceTableText(int inTableID, int inRowCount, AuxilaryDataTableText& inAuxDataTableText);

		bool PushDataToSource() override;
		bool PullDataFromSource();

	private:
		const int m_TableID;
		const int m_TableRowCount;

		AuxilaryDataTableText& m_AuxDataTableText;
	};
}