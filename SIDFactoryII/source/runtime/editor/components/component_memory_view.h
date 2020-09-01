#pragma once

#include "component_base.h"
#include "runtime/editor/datasources/datasource_table_memory_view.h"

#include <memory>
#include <functional>

namespace Foundation
{
	class TextField;
}

namespace Editor
{
	class CursorControl;
	class ScreenBase;

	class ComponentMemoryView final : public ComponentBase
	{
	public:
		static int GetWidthFromColumns(int inColumnCount);

		ComponentMemoryView(int inID, int inGroupID, Undo* inUndo, std::shared_ptr<DataSourceTableMemoryView> inDataSource, Foundation::TextField* inTextField, int inX, int inY, int inHeight);
		~ComponentMemoryView();

		bool ConsumeInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager) override;
		bool ConsumeInput(const Foundation::Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager) override;
		void ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse) override;
		
		void Refresh(const DisplayState& inDisplayState) override;
		void HandleDataChange() override;
		void PullDataFromSource() override;

		void ExecuteInsertDeleteRule(const DriverInfo::TableInsertDeleteRule& inRule, int inSourceTableID, int inIndexPre, int inIndexPost) override;
		void ExecuteAction(int inActionInput) override;
	
	private:
		unsigned short m_MemoryAddress;
		std::shared_ptr<DataSourceTableMemoryView> m_DataSource;
	};
}
