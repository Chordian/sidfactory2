#pragma once

#include "runtime/editor/undo/undo_componentdata.h"

namespace Editor
{
	struct UndoComponentDataTableRowElements : public UndoComponentData
	{
		unsigned int m_TopRow;

		int m_CursorX;
		int m_CursorY;
	};
}