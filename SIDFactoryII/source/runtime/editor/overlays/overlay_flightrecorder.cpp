#include "overlay_flightrecorder.h"

#include "foundation/graphics/viewport.h"
#include "foundation/graphics/textfield.h"
#include "foundation/graphics/drawfield.h"
#include "runtime/editor/components_manager.h"
#include "runtime/editor/components/component_flightrecorder.h"
#include "runtime/editor/datasources/datasource_flightrecorder.h"
#include "runtime/execution/executionhandler.h"
#include "runtime/editor/visualizer_components/vizualizer_component_emulation_state.h"


using namespace Foundation;

namespace Editor
{
	const int OverlayFlightRecorder::ComponentBaseID = 0x100;
	const int OverlayFlightRecorder::ComponentGroupID = 4;


	OverlayFlightRecorder::OverlayFlightRecorder(Foundation::Viewport* inViewport, ComponentsManager* inComponentsManager, Emulation::CPUMemory* inCPUMemory, Emulation::ExecutionHandler* inExecutionHandler, const Foundation::Extent& inMainTextFieldDimensions)
		: m_Viewport(inViewport)
		, m_ComponentsManager(inComponentsManager)
		, m_CPUMemory(inCPUMemory)	
		, m_ExecutionHandler(inExecutionHandler)
		, m_Enabled(false)
	{
		const unsigned int margin_h = 4;
		const unsigned int margin_v = 2;
		const unsigned int width = inMainTextFieldDimensions.m_Width - 2 * margin_h;
		const unsigned int height = inMainTextFieldDimensions.m_Height - 2 * margin_v;
		const unsigned int x = margin_h * Foundation::TextField::font_width;
		const unsigned int y = margin_v * Foundation::TextField::font_height;

		m_TextField = m_Viewport->CreateTextField(width, height, x, y);
		m_TextField->SetEnable(false);

		m_TextField->ColorAreaBackground(Foundation::Color::DarkerBlue);

		AddComponents();
	}


	OverlayFlightRecorder::~OverlayFlightRecorder()
	{
		m_Viewport->Destroy(m_TextField);
		m_Viewport->Destroy(m_DrawField);
	}


	void OverlayFlightRecorder::SetEnabled(bool inEnabled)
	{
		if (inEnabled != m_Enabled)
		{
			m_Enabled = inEnabled;

			m_TextField->SetEnable(inEnabled);
			m_DrawField->SetEnable(inEnabled);
			m_VisualizerEmulationState->SetEnabled(inEnabled);

			m_ComponentsManager->SetGroupEnabledForInput(ComponentGroupID, m_Enabled);
			m_ComponentsManager->SetGroupEnabledForTabbing(m_Enabled ? ComponentGroupID : 0);

			if (m_Enabled)
				m_ComponentsManager->SetComponentInFocus(ComponentBaseID);
		}
	}


	bool OverlayFlightRecorder::IsEnabled() const
	{
		return m_Enabled;
	}


	void OverlayFlightRecorder::AddComponents()
	{
		std::shared_ptr<DataSourceFlightRecorder> data_source_flight_recorder = std::make_shared<DataSourceFlightRecorder>(m_ExecutionHandler->GetFlightRecorder());
		std::shared_ptr<ComponentFlightRecorder> component_flight_recorder = std::make_shared<ComponentFlightRecorder>(
			ComponentBaseID, ComponentGroupID,
			nullptr,
			m_TextField,
			0, 0,
			m_TextField->GetDimensions().m_Height,
			data_source_flight_recorder);

		m_ComponentsManager->AddComponent(component_flight_recorder);

		// Create visualizer draw field
		const auto text_field_position = m_TextField->GetPosition();

		const int offset_x = text_field_position.m_X;
		const int offset_y = text_field_position.m_Y;

		const int draw_field_margin_x = 2 * TextField::font_width;
		const int draw_field_margin_y = TextField::font_height;

		const int draw_field_width = 300;
		const int draw_field_height = 150;
 		const int draw_field_x = offset_x + m_TextField->GetDimensions().m_Width * TextField::font_width - draw_field_width - draw_field_margin_x;
		const int draw_field_y = offset_y + m_TextField->GetDimensions().m_Height * TextField::font_height - draw_field_height - draw_field_margin_y;

		m_DrawField = m_Viewport->CreateDrawField(draw_field_width, draw_field_height, draw_field_x, draw_field_y);
		m_DrawField->SetEnable(false);

		// Add components to visualizer draw field
		m_VisualizerEmulationState = std::make_shared<VisualizerComponentEmulationState>(
			0,
			m_DrawField,
			0,
			0,
			draw_field_width,
			draw_field_height,
			data_source_flight_recorder);

		m_VisualizerEmulationState->SetEnabled(false);

		m_ComponentsManager->AddVisualizerComponent(m_VisualizerEmulationState);
	}
}