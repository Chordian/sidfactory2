#pragma once

#include "runtime/editor/undo/undo_datasource/undo_datasource.h"

namespace Editor
{
	class UndoComponentData
	{
	public:
		virtual UndoDataSource& GetDataSource() = 0;
		virtual const UndoDataSource& GetDataSource() const = 0;

		int m_ComponentID;
		int m_ComponentGroupID;
	};
}