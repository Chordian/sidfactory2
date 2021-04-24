#include "runtime/editor/undo/undo.h"
#include "runtime/editor/undo/undostep.h"
#include "runtime/editor/undo/undo_componentdata/undo_componentdata.h"
#include "runtime/editor/undo/undo_datasource/undo_datasource.h"
#include "runtime/editor/undo/undo_datasource/undo_datasource.h"
#include "runtime/editor/driver/driver_info.h"
#include "runtime/editor/auxilarydata/auxilary_data_collection.h"
#include "runtime/editor/auxilarydata/auxilary_data_table_text.h"
#include "runtime/emulation/cpumemory.h"
#include <memory>
#include "foundation/base/assert.h"
#include "foundation/graphics/textfield.h"

namespace Editor
{
	Undo::Undo(Emulation::CPUMemory& inCPUMemory, DriverInfo& inDriverInfo)
		: m_CPUMemory(inCPUMemory)
		, m_DriverInfo(inDriverInfo)
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
		//for (auto& step : m_UndoStepsRecentEdits)
		//	step = nullptr;

		m_Begin = 0;
		m_End = 0;
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
		auto& data_source = inComponentUndoData->GetDataSource();

		if (inLockCPU)
			m_CPUMemory.Lock();

		GetDataForUndo(data_source);
		//m_CPUMemory.GetData(m_DataSnapshotAddressBegin, static_cast<void*>(data), m_DataSnapshotSize);

		if (inLockCPU)
			m_CPUMemory.Unlock();

		m_UndoSteps[m_End] = std::make_shared<UndoStep>(data, inComponentUndoData, inRestorePostFunction);

		FlushForwardUndoSteps();
	}


	void Undo::AddUndo(const std::shared_ptr<UndoComponentData>& inComponentUndoData, std::function<void(const UndoComponentData&, CursorControl&)> inRestorePostFunction)
	{
		FOUNDATION_ASSERT(m_End < m_UndoSteps.size());

		unsigned char* data = new unsigned char[m_DataSnapshotSize];
		auto& data_source = inComponentUndoData->GetDataSource();

		m_CPUMemory.Lock();
		GetDataForUndo(data_source);
		//m_CPUMemory.GetData(m_DataSnapshotAddressBegin, static_cast<void*>(data), m_DataSnapshotSize);
		m_CPUMemory.Unlock();

		//m_UndoStepsRecentEdits[m_End] = m_UndoSteps[m_End];
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


	void Undo::FlushForwardUndoSteps()
	{
		unsigned int i = m_End + 1;

		while (true)
		{
			if (i == m_UndoSteps.size())
				i = 0;
			if (i == m_Begin)
				break;

			m_UndoSteps[i] = nullptr;
			//m_UndoStepsRecentEdits[i] = nullptr;

			++i;
		}
	}


	int Undo::DoUndo(CursorControl& inCursorControl)
	{
		FOUNDATION_ASSERT(HasUndoStep());

		int new_end = static_cast<int>(m_End) - 1;

		if (new_end < 0)
			new_end = static_cast<int>(m_UndoSteps.size() - 1);

		FOUNDATION_ASSERT(m_UndoSteps[new_end] != nullptr);

		const unsigned char* restore_data = m_UndoSteps[new_end]->GetData();
		//const unsigned char* resent_edit_restore_data = m_UndoStepsRecentEdits[new_end] != nullptr ? m_UndoStepsRecentEdits[new_end]->GetData() : nullptr;
		const unsigned char* resent_edit_restore_data = nullptr; 

		if (restore_data != nullptr || resent_edit_restore_data != nullptr)
		{
			m_CPUMemory.Lock();

//			if (resent_edit_restore_data != nullptr)
//			{
//				const auto& component_data = m_UndoStepsRecentEdits[new_end]->GetComponentData();
//				const auto& data_source = component_data.GetDataSource();
//				RestoreDataFromUndo(data_source);
//
//				//m_CPUMemory.SetData(m_DataSnapshotAddressBegin, static_cast<const void*>(resent_edit_restore_data), m_DataSnapshotSize);
//			}
			if (restore_data != nullptr)
			{
				const auto& component_data = m_UndoSteps[new_end]->GetComponentData();
				const auto& data_source = component_data.GetDataSource();
				RestoreDataFromUndo(data_source);

				//m_CPUMemory.SetData(m_DataSnapshotAddressBegin, static_cast<const void*>(restore_data), m_DataSnapshotSize);
			}

			m_CPUMemory.Unlock();
		}

		const int component_id = m_UndoSteps[new_end]->GetComponentData().m_ComponentID;
		const int component_group_id = m_UndoSteps[new_end]->GetComponentData().m_ComponentGroupID;

		if (m_RestoredStepComponentHandler != nullptr)
			m_RestoredStepComponentHandler(component_id, component_group_id);

		//if (m_UndoStepsRecentEdits[new_end] != nullptr)
		//	m_UndoStepsRecentEdits[new_end]->OnRestored(inCursorControl);

		m_UndoSteps[new_end]->OnRestored(inCursorControl);

		m_End = static_cast<unsigned int>(new_end);

		return component_id;
	}


	int Undo::DoRedo(CursorControl& inCursorControl)
	{
		FOUNDATION_ASSERT(HasRedoStep());

		unsigned int new_end = m_End + 1;

		if (new_end >= m_UndoSteps.size())
			new_end = 0;

		FOUNDATION_ASSERT(m_UndoSteps[new_end] != nullptr);

		const unsigned char* restore_data = m_UndoSteps[new_end]->GetData();
		//const unsigned char* resent_edit_restore_data = m_UndoStepsRecentEdits[new_end] != nullptr ? m_UndoStepsRecentEdits[new_end]->GetData() : nullptr;
		const unsigned char* resent_edit_restore_data = nullptr;

		if (restore_data != nullptr || resent_edit_restore_data != nullptr)
		{
			m_CPUMemory.Lock();

//			if (resent_edit_restore_data != nullptr)
//			{
//				const auto& component_data = m_UndoStepsRecentEdits[new_end]->GetComponentData();
//				const auto& data_source = component_data.GetDataSource();
//				RestoreDataFromUndo(data_source);
//
//				//m_CPUMemory.SetData(m_DataSnapshotAddressBegin, static_cast<const void*>(resent_edit_restore_data), m_DataSnapshotSize);
//			}
			if (restore_data != nullptr)
			{
				const auto& component_data = m_UndoSteps[new_end]->GetComponentData();
				const auto& data_source = component_data.GetDataSource();
				RestoreDataFromUndo(data_source);

				//m_CPUMemory.SetData(m_DataSnapshotAddressBegin, static_cast<const void*>(restore_data), m_DataSnapshotSize);
			}

			m_CPUMemory.Unlock();
		}

		const int component_id = m_UndoSteps[new_end]->GetComponentData().m_ComponentID;
		const int component_group_id = m_UndoSteps[new_end]->GetComponentData().m_ComponentGroupID;

		if (m_RestoredStepComponentHandler != nullptr)
			m_RestoredStepComponentHandler(component_id, component_group_id);

		//if (m_UndoStepsRecentEdits[new_end] != nullptr)
		//	m_UndoStepsRecentEdits[new_end]->OnRestored(inCursorControl);

		m_UndoSteps[new_end]->OnRestored(inCursorControl);

		m_End = new_end;

		return component_id;
	}


	void Undo::PrintDebug(Foundation::TextField& inTextField)
	{
		inTextField.PrintHexValue(0, 0, false, static_cast<unsigned short>(m_Begin));
		inTextField.PrintHexValue(0, 1, false, static_cast<unsigned short>(m_End));

		for (size_t i = 0; i < m_UndoSteps.size(); ++i)
		{
			std::string text = std::to_string(reinterpret_cast<unsigned long>(m_UndoSteps[i].get()));
			inTextField.Print(5, i, Foundation::Color::White, text);
		}
	}


	void Undo::GetDataForUndo(UndoDataSource& inData)
	{
		// CPU Memory
		unsigned char* data = new unsigned char[m_DataSnapshotSize];
		m_CPUMemory.GetData(m_DataSnapshotAddressBegin, static_cast<void*>(data), m_DataSnapshotSize);
		inData.SetCPUMemoryData(data);

		// Auxilary data table text
		const auto& table_text = m_DriverInfo.GetAuxilaryDataCollection().GetTableText();
		inData.SetAuxilaryDataTableText(table_text);
	}


	void Undo::RestoreDataFromUndo(const UndoDataSource& inData)
	{
		// CPU Memory
		m_CPUMemory.SetData(m_DataSnapshotAddressBegin, static_cast<const void*>(inData.GetCPUMemoryData()), m_DataSnapshotSize);

		// Auxilary data table text
		auto& table_text = m_DriverInfo.GetAuxilaryDataCollection().GetTableText();
		table_text = inData.GetAuxilaryDataTableText();
	}
}