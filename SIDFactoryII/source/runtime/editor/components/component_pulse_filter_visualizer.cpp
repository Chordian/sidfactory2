#include "runtime/editor/components/component_pulse_filter_visualizer.h"

#include "foundation/graphics/drawfield.h"
#include "foundation/graphics/textfield.h"
#include "foundation/graphics/viewport.h"
#include "runtime/editor/components_manager.h"
#include "runtime/editor/datasources/datasource_sidregistersbuffer.h"
#include "runtime/editor/visualizer_components/visualizer_component_pulse_filter_state.h"
#include "runtime/execution/executionhandler.h"
#include "runtime/editor/datasources/datasource_track_components.h"

namespace Editor
{
	ComponentPulseFilterVisualizer::ComponentPulseFilterVisualizer(
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
		int inHeight)
		: ComponentBase(inID, inGroupID, inUndo, inTextField, inX, inY, inWidth, inHeight)
		, m_Viewport(inViewport)
		, m_ComponentsManager(inComponentsManager)
	{
		unsigned int viewport_width = inWidth * Foundation::TextField::font_width;
		unsigned int viewport_height = inHeight * Foundation::TextField::font_height;

		m_DrawField = m_Viewport->CreateDrawField(viewport_width, viewport_height, inX * Foundation::TextField::font_width, inY * Foundation::TextField::font_height);
		m_DrawField->SetEnable(true);

		std::shared_ptr<DataSourceSIDRegistersBufferAfLastDriverUpdate> data_source_sid_registers_buffer = std::make_shared<DataSourceSIDRegistersBufferAfLastDriverUpdate>(inExecutionHandler);

		m_VisualizerComponent = std::make_shared<VisualizerComponentPulseFilterState>(1, m_DrawField, 0, 0, viewport_width, viewport_height, data_source_sid_registers_buffer, inTracks);
		m_VisualizerComponent->SetEnabled(true);
		m_ComponentsManager->AddVisualizerComponent(m_VisualizerComponent);
	}

	ComponentPulseFilterVisualizer::~ComponentPulseFilterVisualizer()
	{
		m_Viewport->Destroy(m_DrawField);
	}


	bool ComponentPulseFilterVisualizer::ConsumeInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		return false;
	}

	bool ComponentPulseFilterVisualizer::ConsumeInput(const Foundation::Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		return false;
	}

	bool ComponentPulseFilterVisualizer::ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse)
	{
		return false;
	}

	void ComponentPulseFilterVisualizer::Refresh(const DisplayState& inDisplayState)
	{
	}

	void ComponentPulseFilterVisualizer::HandleDataChange()
	{
	}

	void ComponentPulseFilterVisualizer::PullDataFromSource(const bool inFromUndo)
	{
	}

	void ComponentPulseFilterVisualizer::ExecuteInsertDeleteRule(const DriverInfo::TableInsertDeleteRule& inRule, int inSourceTableID, int inIndexPre, int inIndexPost)
	{
	}

	void ComponentPulseFilterVisualizer::ExecuteAction(int inActionInput)
	{
	}

}
