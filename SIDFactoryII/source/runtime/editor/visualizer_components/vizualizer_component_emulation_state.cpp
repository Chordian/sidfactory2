#include "vizualizer_component_emulation_state.h"

#include "foundation/graphics/drawfield.h"
#include "runtime/editor/datasources/datasource_flightrecorder.h"
#include "runtime/environmentdefines.h"

using namespace Foundation;

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
			m_DrawField->DrawBox(Color::DarkGrey, 0, 0, m_Dimensions.m_Width, m_Dimensions.m_Height);

			m_DataSource->Lock();

			const auto fetch_cycle_value = [&](int inIndex) -> const int
			{
				const int cycle_count = (*m_DataSource)[inIndex].m_nCyclesSpend;
				const int scan_lines = cycle_count / EMULATION_CYCLES_PER_SCANLINE_PAL;
				return 4 * scan_lines;
			};

			const auto fetch_cycle_color = [](const int inValue) -> Foundation::Color
			{
				if (inValue < 16 * 4)
					return Color::Green;
				if (inValue < 24 * 4)
					return Color::Yellow;

				return Color::Red;
			};

			DrawColoredFilled(fetch_cycle_value, fetch_cycle_color);
/*
			const auto fetch_pulse_value = [&](int inIndex) -> const int
			{
				const int pulse_width = (static_cast<int>((*m_DataSource)[inIndex].m_SIDData[0x03]) << 8) + static_cast<int>((*m_DataSource)[inIndex].m_SIDData[0x02]);
				return (pulse_width & 0xfff) >> 5;
			};

			const auto fetch_pulse_color = [&](int inValue)
			{
				return Color::Green;
			};

			DrawColoredFilled(fetch_pulse_value, fetch_pulse_color);
*/
			for (int i = 0; i < 8; ++i)
			{
				int y = (1 + i) * 16;
				m_DrawField->DrawHorizontalLine((i & 1) == 1 ? Color::White : Color::DarkGrey, 0, m_Dimensions.m_Width, m_Dimensions.m_Height - y);
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
			const int value = inValueFunction(index);
			const Color color = inColorFunction(value);

			m_DrawField->DrawVerticalLine(color, m_Dimensions.m_Width - 1 - i, m_Dimensions.m_Height - 1 - value, m_Dimensions.m_Height - 1);

			--index;
			if (index >= size)
				index = size - 1;
		}
	}
}
