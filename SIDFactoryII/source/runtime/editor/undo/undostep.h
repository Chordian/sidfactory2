#pragma once

#include <functional>
#include <memory>

namespace Editor
{
	struct UndoComponentData;
	class CursorControl;
	class UndoStep
	{
	public:
		UndoStep() = delete;
		UndoStep(unsigned char* inData, const std::shared_ptr<UndoComponentData>& inComponentUndoData, std::function<void(const UndoComponentData&, CursorControl&)> inRestorePostFunction);

		~UndoStep();

		const unsigned char* GetData() const;
		void OnRestored(CursorControl& inCursorControl);

		const UndoComponentData& GetComponentData() const;

	private:
		unsigned char* m_Data;

		std::shared_ptr<UndoComponentData> m_ComponentData;
		std::function<void(const UndoComponentData&, CursorControl&)> m_RestorePostExecution;
	};
}