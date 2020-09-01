#pragma once

#include <array>
#include <memory>
#include <functional>

namespace Emulation
{
	class CPUMemory;
}

namespace Editor
{
	class DriverInfo;
	class UndoStep;
	class CursorControl;
	struct UndoComponentData;

	class Undo final
	{
	public:
		Undo(Emulation::CPUMemory& inCPUMemory, const DriverInfo& inDriverInfo);

		void Clear();
		void SetOnRestoredStepComponentHandler(std::function<void(int, int)> inHandler);

		bool HasUndoStep() const;
		bool HasRedoStep() const;

		void AddMostRecentEdit(bool inLockCPU, const std::shared_ptr<UndoComponentData>& inComponentUndoData, std::function<void(const UndoComponentData&, CursorControl&)> inRestorePostFunction);
		void AddUndo(const std::shared_ptr<UndoComponentData>& inComponentUndoData, std::function<void(const UndoComponentData&, CursorControl&)> inRestorePostFunction);
		void DoUndo(CursorControl& inCursorControl);
		void DoRedo(CursorControl& inCursorControl);
	
	private:
		unsigned int m_Begin;
		unsigned int m_End;

		unsigned short m_DataSnapshotAddressBegin;
		unsigned short m_DataSnapshotSize;

		Emulation::CPUMemory& m_CPUMemory;

		std::array<std::shared_ptr<UndoStep>, 256> m_UndoSteps;
		std::function<void(int, int)> m_RestoredStepComponentHandler;
	};
}