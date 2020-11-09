#include "vizualizer_component_emulation_state.h"

#include "foundation/graphics/drawfield.h"
#include "runtime/editor/datasources/datasource_flightrecorder.h"
#include "runtime/environmentdefines.h"
#include "utils/usercolors.h"

using namespace Foundation;
using namespace Utility;

namespace Editor
{
	VisualizerComponentEmulationState::VisualizerComponentEmulationState(
		int inID,
		Foundation::DrawField* inDrawField,
		int inX,
		int inY,
		int inWidth,
		int inHeight,
		std::shared_ptr<DataSourceFlightRecorder> inDataSource
	)
		: VisualizerComponentBase(inID, inDrawField, inX, inY, inWidth, inHeight)
		, m_DataSource(inDataSource)
	{

	}


	VisualizerComponentEmulationState::~VisualizerComponentEmulationState()
	{

	}


	void VisualizerComponentEmulationState::ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse)
	{

	}


	void VisualizerComponentEmulationState::Refresh(const DisplayState& inDisplayState)
	{
		if (m_Enabled)
		{
			const Color color_cpu_usage_background = ToColor(UserColor::FlightRecorderVisualizerBackground);
			const Color color_cpu_usage_horizontal_line_1 = ToColor(UserColor::FlightRecorderVisualizerHorizontalLine1);
			const Color color_cpu_usage_horizontal_line_2 = ToColor(UserColor::FlightRecorderVisualizerHorizontalLine2);
			const Color color_cpu_usage_low = ToColor(UserColor::FlightRecorderVisualizerCPUUsageLow);
			const Color color_cpu_usage_medium = ToColor(UserColor::FlightRecorderVisualizerCPUUsageMedium);
			const Color color_cpu_usage_high = ToColor(UserColor::FlightRecorderVisualizerCPUUsageHigh);

			m_DrawField->DrawBox(color_cpu_usage_background, 0, 0, m_Dimensions.m_Width, m_Dimensions.m_Height);

			m_DataSource->Lock();

			const auto fetch_cycle_value = [&](int inIndex) -> const int
			{
				const int cycle_count = (*m_DataSource)[inIndex].m_nCyclesSpend;
				const int scan_lines = cycle_count / EMULATION_CYCLES_PER_SCANLINE_PAL;
				return scan_lines;
			};

			const auto fetch_cycle_color = [&](const int inValue) -> const Color
			{
				if (inValue < 0x10)
					return color_cpu_usage_low;
				if (inValue < 0x18)
					return color_cpu_usage_medium;

				return color_cpu_usage_high;
			};

			DrawColoredFilled(fetch_cycle_value, fetch_cycle_color);

			for (int i = 0; i < 8; ++i)
			{
				int y = (1 + i) * 16;
				m_DrawField->DrawHorizontalLine((i & 1) == 1 ? color_cpu_usage_horizontal_line_1 : color_cpu_usage_horizontal_line_2, 0, m_Dimensions.m_Width, m_Dimensions.m_Height - y);
			}

			m_DataSource->Unlock();
		}
	}


	void VisualizerComponentEmulationState::DrawColoredFilled(
		const std::function<const int(unsigned int)>& inValueFunction,
		const std::function<Foundation::Color(const int)>& inColorFunction)
	{
		unsigned int index = m_DataSource->GetNewestRecordingIndex();
		unsigned int size = static_cast<unsigned int>(m_DataSource->GetSize());

		for (int i = 0; i < m_Dimensions.m_Width; ++i)
		{
			const int scan_lines = inValueFunction(index);
			const Color color = inColorFunction(scan_lines);
			const int value = scan_lines * 4;

			m_DrawField->DrawVerticalLine(color, m_Dimensions.m_Width - 1 - i, m_Dimensions.m_Height - 1 - value, m_Dimensions.m_Height - 1);

			--index;
			if (index >= size)
				index = size - 1;
		}
	}
}
