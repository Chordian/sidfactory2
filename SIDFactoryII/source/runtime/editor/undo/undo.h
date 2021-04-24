#pragma once

#include <array>
#include <memory>
#include <functional>

namespace Foundation
{
	class TextField;
}

namespace Emulation
{
	class CPUMemory;
}

namespace Editor
{
	class DriverInfo;
	class UndoStep;
	class CursorControl;
	class UndoComponentData;
	class UndoDataSource;

	class Undo final
	{
	public:
		Undo(Emulation::CPUMemory& inCPUMemory, DriverInfo& inDriverInfo);

		void Clear();
		void SetOnRestoredStepComponentHandler(std::function<void(int, int)> inHandler);

		bool HasUndoStep() const;
		bool HasRedoStep() const;

		void AddMostRecentEdit(bool inLockCPU, const std::shared_ptr<UndoComponentData>& inComponentUndoData, std::function<void(const UndoComponentData&, CursorControl&)> inRestorePostFunction);
		void AddUndo(const std::shared_ptr<UndoComponentData>& inComponentUndoData, std::function<void(const UndoComponentData&, CursorControl&)> inRestorePostFunction);
		void FlushForwardUndoSteps();

		int DoUndo(CursorControl& inCursorControl);
		int DoRedo(CursorControl& inCursorControl);

		void PrintDebug(Foundation::TextField& inTextField);
	
	private:
		void GetDataForUndo(UndoDataSource& inData);
		void RestoreDataFromUndo(const UndoDataSource& inData);

		unsigned int m_Begin;
		unsigned int m_End;

		unsigned short m_DataSnapshotAddressBegin;
		unsigned short m_DataSnapshotSize;

		Emulation::CPUMemory& m_CPUMemory;
		DriverInfo& m_DriverInfo;

		std::array<std::shared_ptr<UndoStep>, 256> m_UndoSteps;
		//std::array<std::shared_ptr<UndoStep>, 256> m_UndoStepsRecentEdits;

		std::function<void(int, int)> m_RestoredStepComponentHandler;
	};
}