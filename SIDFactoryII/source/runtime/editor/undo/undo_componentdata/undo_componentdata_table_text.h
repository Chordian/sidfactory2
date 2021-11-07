#pragma once

#include "runtime/editor/undo/undo_componentdata/undo_componentdata.h"
#include "runtime/editor/undo/undo_datasource/undo_datasource.h"
#include <vector>
#include <string>

namespace Editor
{
	class UndoComponentDataTableText : public UndoComponentData
	{
	public:
		std::vector<std::string> m_TextLines;

		UndoDataSource& GetDataSource() override { return m_Data; }
		const UndoDataSource& GetDataSource() const override { return m_Data; }

	private:
		UndoDataSource m_Data;
	};
}