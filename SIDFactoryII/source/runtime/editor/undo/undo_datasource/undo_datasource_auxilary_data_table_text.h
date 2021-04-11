#pragma once

#include "source/runtime/editor/undo/undo_datasource/undo_datasource.h"
#include "source/runtime/editor/auxilarydata/auxilary_data_table_text.h"

namespace Editor
{
	class UndoDataSourceAuxilaryDataTableText : public UndoDataSource
	{
	public:
		UndoDataSourceAuxilaryDataTableText()
		{

		}

		~UndoDataSourceAuxilaryDataTableText()
		{
		}

		Type GetType() const override { return Type::AuxilaryDataTableText; }


		void Set(const AuxilaryDataTableText& inSource)
		{
			m_Data = inSource;
		}

		void Restore(AuxilaryDataTableText& inDestination) const
		{
			inDestination = m_Data;
		}

	private:
		AuxilaryDataTableText m_Data;
	};
}
