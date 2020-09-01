#include "runtime/editor/undo/undostep.h"
#include <assert.h>

namespace Editor
{
	UndoStep::UndoStep(unsigned char* inData, const std::shared_ptr<UndoComponentData>& inComponentUndoData, std::function<void(const UndoComponentData&, CursorControl&)> inRestorePostFunction)
		: m_Data(inData)
		, m_ComponentData(inComponentUndoData)
		, m_RestorePostExecution(inRestorePostFunction)
	{
		assert(inData != nullptr);
	}

	UndoStep::~UndoStep()
	{
        delete[] m_Data;
	}

	const unsigned char* UndoStep::GetData() const
	{
		assert(m_Data != nullptr);
		return m_Data;
	}


	void UndoStep::OnRestored(CursorControl& inCursorControl)
	{
		m_RestorePostExecution(*m_ComponentData, inCursorControl);
	}


	const UndoComponentData& UndoStep::GetComponentData() const
	{
		return *m_ComponentData;
	}
}
