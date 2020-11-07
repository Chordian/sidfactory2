#include "runtime/editor/undo/undo.h"
#include "runtime/editor/undo/undostep.h"
#include "runtime/editor/undo/undo_componentdata.h"
#include "runtime/editor/driver/driver_info.h"
#include "runtime/emulation/cpumemory.h"
#include <memory>
#include "foundation/base/assert.h"

namespace Editor
{
	Undo::Undo(Emulation::CPUMemory& inCPUMemory, const DriverInfo& inDriverInfo)
		: m_CPUMemory(inCPUMemory)
		, m_DataSnapshotAddressBegin(inDriverInfo.GetDescriptor().m_DriverCodeTop + inDriverInfo.GetDescriptor().m_DriverSize)
		, m_DataSnapshotSize(0x10000 - m_DataSnapshotAddressBegin)
		, m_Begin(0)
		, m_End(0)
	{
	}


	void Undo::Clear()
	{
		for (auto& step : m_UndoSteps)
			step = nullptr;
	}


	void Undo::SetOnRestoredStepComponentHandler(std::function<void(int, int)> inHandler)
	{
		m_RestoredStepComponentHandler = inHandler;
	}


	bool Undo::HasUndoStep() const
	{
		return m_Begin != m_End;
	}


	bool Undo::HasRedoStep() const
	{
		unsigned int next_end = m_End + 1;
		
		if (next_end >= m_UndoSteps.size())
			next_end = 0;

		if (m_Begin == next_end)
			return false;

		return m_UndoSteps[next_end] != nullptr;
	}


	void Undo::AddMostRecentEdit(bool inLockCPU, const std::shared_ptr<UndoComponentData>& inComponentUndoData, std::function<void(const UndoComponentData&, CursorControl&)> inRestorePostFunction)
	{
		FOUNDATION_ASSERT(m_End < m_UndoSteps.size());

		unsigned char* data = new unsigned char[m_DataSnapshotSize];

		if(inLockCPU)
			m_CPUMemory.Lock();
		
		m_CPUMemory.GetData(m_DataSnapshotAddressBegin, static_cast<void*>(data), m_DataSnapshotSize);
	
		if(inLockCPU)
			m_CPUMemory.Unlock();

		m_UndoSteps[m_End] = std::make_shared<UndoStep>(data, inComponentUndoData, inRestorePostFunction);

		// Flush forward
		unsigned int i = m_End + 1;

		while (true)
		{
			if (i == m_UndoSteps.size())
				i = 0;
			if (i == m_Begin)
				break;

			m_UndoSteps[i] = nullptr;

			++i;
		}
	}


	void Undo::AddUndo(const std::shared_ptr<UndoComponentData>& inComponentUndoData, std::function<void(const UndoComponentData&, CursorControl&)> inRestorePostFunction)
	{
		FOUNDATION_ASSERT(m_End < m_UndoSteps.size());

		unsigned char* data = new unsigned char[m_DataSnapshotSize];

		m_CPUMemory.Lock();
		m_CPUMemory.GetData(m_DataSnapshotAddressBegin, static_cast<void*>(data), m_DataSnapshotSize);
		m_CPUMemory.Unlock();

		m_UndoSteps[m_End] = std::make_shared<UndoStep>(data, inComponentUndoData, inRestorePostFunction);

		++m_End;
		if (m_End == m_UndoSteps.size())
			m_End = 0;

		if (m_End == m_Begin)
		{
			++m_Begin;
			if (m_Begin == m_UndoSteps.size())
				m_Begin = 0;
		}
	}


	void Undo::DoUndo(CursorControl& inCursorControl)
	{
		FOUNDATION_ASSERT(HasUndoStep());

		int new_end = static_cast<int>(m_End) - 1;

		if (new_end < 0)
			new_end = static_cast<int>(m_UndoSteps.size() - 1);

		FOUNDATION_ASSERT(m_UndoSteps[new_end] != nullptr);

		const unsigned char* restore_data = m_UndoSteps[new_end]->GetData();

		if(restore_data != nullptr)
		{
			m_CPUMemory.Lock();
			m_CPUMemory.SetData(m_DataSnapshotAddressBegin, static_cast<const void*>(restore_data), m_DataSnapshotSize);
			m_CPUMemory.Unlock();
		}

		const int component_id = m_UndoSteps[new_end]->GetComponentData().m_ComponentID;
		const int component_group_id = m_UndoSteps[new_end]->GetComponentData().m_ComponentGroupID;

		if (m_RestoredStepComponentHandler != nullptr)
			m_RestoredStepComponentHandler(component_id, component_group_id);

		m_UndoSteps[new_end]->OnRestored(inCursorControl);

		m_End = static_cast<unsigned int>(new_end);
	}


	void Undo::DoRedo(CursorControl& inCursorControl)
	{
		FOUNDATION_ASSERT(HasRedoStep());

		unsigned int new_end = m_End + 1;

		if (new_end >= m_UndoSteps.size())
			new_end = 0;

		FOUNDATION_ASSERT(m_UndoSteps[new_end] != nullptr);

		const unsigned char* restore_data = m_UndoSteps[new_end]->GetData();

		if (restore_data != nullptr)
		{
			m_CPUMemory.Lock();
			m_CPUMemory.SetData(m_DataSnapshotAddressBegin, static_cast<const void*>(restore_data), m_DataSnapshotSize);
			m_CPUMemory.Unlock();
		}

		const int component_id = m_UndoSteps[new_end]->GetComponentData().m_ComponentID;
		const int component_group_id = m_UndoSteps[new_end]->GetComponentData().m_ComponentGroupID;

		if (m_RestoredStepComponentHandler != nullptr)
			m_RestoredStepComponentHandler(component_id, component_group_id);

		m_UndoSteps[new_end]->OnRestored(inCursorControl);

		m_End = new_end;
	}
}
