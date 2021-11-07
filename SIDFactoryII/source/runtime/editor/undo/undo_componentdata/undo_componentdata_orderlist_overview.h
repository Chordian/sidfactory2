#pragma once

#include "runtime/editor/undo/undo_componentdata/undo_componentdata.h"
#include "runtime/editor/undo/undo_datasource/undo_datasource.h"

namespace Editor
{
	class UndoComponentDataOrderListOverview : public UndoComponentData
	{
	public:		
		unsigned int m_TopPosition;

		int m_CursorX;
		int m_CursorY;

		UndoDataSource& GetDataSource() override { return m_Data; }
		const UndoDataSource& GetDataSource() const override { return m_Data; }

	private:
		UndoDataSource m_Data;
	};
}