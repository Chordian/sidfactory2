#pragma once
#include "component_base.h"
#include "runtime/editor/datasources/datasource_track_components.h"

namespace Emulation
{
	class ExecutionHandler;
}

namespace Editor
{
	class DataSourceFlightRecorder;
	class VisualizerComponentPulseFilterState;
}

namespace Foundation
{
	class DrawField;
}

namespace Foundation
{
	class Viewport;
}

namespace Editor
{
	class ComponentPulseFilterVisualizer : public ComponentBase
	{
	  public:
		ComponentPulseFilterVisualizer(
			int inID,
			int inGroupID,
			Undo* inUndo,
			Emulation::ExecutionHandler* inExecutionHandler,
			Foundation::TextField* inTextField,
			Foundation::Viewport* inViewport,
			ComponentsManager* inComponentsManager,
			std::shared_ptr<DataSourceTrackComponents> inTracks,
			int inX,
			int inY,
			int inWidth,
			int inHeight);
		virtual ~ComponentPulseFilterVisualizer();

		bool CanReceiveFocus() const override { return false; }

		bool ConsumeInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager) override;
		bool ConsumeInput(const Foundation::Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager) override;
		bool ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse) override;

		void Refresh(const DisplayState& inDisplayState) override;
		void HandleDataChange() override;
		void PullDataFromSource(const bool inFromUndo) override;

		void ExecuteInsertDeleteRule(const DriverInfo::TableInsertDeleteRule& inRule, int inSourceTableID, int inIndexPre, int inIndexPost) override;
		void ExecuteAction(int inActionInput) override;

	  private:
		Foundation::Viewport* m_Viewport;
		Foundation::DrawField* m_DrawField;

		ComponentsManager* m_ComponentsManager;

		Emulation::ExecutionHandler* m_ExecutionHandler;

		std::shared_ptr<VisualizerComponentPulseFilterState> m_VisualizerComponent;
	};
}
