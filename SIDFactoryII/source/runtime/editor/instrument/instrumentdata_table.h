#pragma once

namespace Editor
{
	class InstrumentDataTable
	{
	public:
		InstrumentDataTable(unsigned char inID, unsigned int inColumnCount, unsigned int inRowCount);
		~InstrumentDataTable();

	private:
		unsigned char m_ID;
		unsigned int m_ColumnCount;
		unsigned int m_RowCount;
	};
}