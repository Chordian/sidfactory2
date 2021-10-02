#pragma once

#include "component_base.h"

#include <memory>
#include <string>
#include <functional>

namespace Foundation
{
	class TextField;
}

namespace Editor
{
	class CursorControl;
	class ScreenBase;
	class DataSourceFlightRecorder;

	class ComponentFlightRecorder final : public ComponentBase
	{
	public:
		ComponentFlightRecorder(int inID, int inGroupID, Undo* inUndo, Foundation::TextField* inTextField, int inX, int inY, int inHeight, std::shared_ptr<DataSourceFlightRecorder>& inDataSource);
		~ComponentFlightRecorder();

		void SetHasControl(GetControlType inGetControlType, CursorControl& inCursorControl) override;

		bool ConsumeInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager) override;
		bool ConsumeInput(const Foundation::Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager) override;
		void ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse) override;

		void Refresh(const DisplayState& inDisplayState) override;
		void HandleDataChange() override;
		void PullDataFromSource(const bool inFromUndo) override;

		void ExecuteInsertDeleteRule(const DriverInfo::TableInsertDeleteRule& inRule, int inSourceTableID, int inIndexPre, int inIndexPost) override;
		void ExecuteAction(int inActionInput) override;

	private:
		std::shared_ptr<DataSourceFlightRecorder> m_DataSource;

		unsigned int m_CursorPos;
		unsigned int m_MaxCursorPos;
		unsigned int m_TopVisible;
	};
}
