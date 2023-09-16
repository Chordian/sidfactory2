#include "visualizer_component_pulse_filter_state.h"

#include "foundation/graphics/drawfield.h"
#include "foundation/input/mouse.h"
#include "runtime/editor/components/component_file_selector.h"
#include "runtime/editor/datasources/datasource_sidregistersbuffer.h"
#include "runtime/execution/executionhandler.h"
#include "runtime/execution/flightrecorder.h"
#include "utils/configfile.h"
#include "utils/global.h"
#include "utils/usercolors.h"

using namespace Foundation;
using namespace Utility;
using namespace Utility::Config;

namespace Editor
{
	VisualizerComponentPulseFilterState::VisualizerComponentPulseFilterState(
		int inID,
		Foundation::DrawField* inDrawField,
		int inX,
		int inY,
		int inWidth,
		int inHeight,
		std::shared_ptr<DataSourceSIDRegistersBufferAfLastDriverUpdate> inDataSource,
		std::shared_ptr<DataSourceTrackComponents> inTracks)
		: VisualizerComponentBase(inID, inDrawField, inX, inY, inWidth, inHeight)
		, m_DataSource(inDataSource)
		, m_Tracks(inTracks)
	{
		ConfigFile& config_file = Global::instance().GetConfig();
		m_PulseWidthStyle = GetSingleConfigurationValue<ConfigValueInt>(config_file, "Visualizer.PulseWidth.Style", 0);
	}


	VisualizerComponentPulseFilterState::~VisualizerComponentPulseFilterState()
	{
	}


	bool VisualizerComponentPulseFilterState::ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse)
	{
		if (inMouse.IsButtonPressed(Mouse::Button::Left))
		{
			m_PulseWidthStyle = (m_PulseWidthStyle + 1) & 1;
			return true;
		}

		return false;
	}


	void VisualizerComponentPulseFilterState::Refresh(const DisplayState& inDisplayState)
	{
		if (m_Enabled)
		{
			const Color color_background = ToColor(UserColor::StateBarArea);
			const Color color_bar = ToColor(UserColor::TrackBackground);
			const Color color_bar_filtered_channel = ToColor(UserColor::StateBarBackgroundFilteredChannel);
			const Color color_bar_fill = ToColor(UserColor::StateBarFillColorPulse);
			const Color color_bar_fill_filter = ToColor(UserColor::StateBarFillColorFilter);
			const Color color_muted = ToColor(UserColor::TrackBackgroundMuted);

			m_DataSource->PullDataFromSource();

			m_DrawField->DrawBox(color_background, 0, 0, m_Dimensions.m_Width, m_Dimensions.m_Height);

			const int bar_width = m_Dimensions.m_Width - 4;
			const int bar_height = 12;
			const int bar_spacing = 16;

			int bar_x = 2;
			int bar_y = 2;

			const auto& data_source = *m_DataSource;

			const auto get_pulse_value = [&data_source](unsigned int inChannel) -> unsigned short
			{
				if (inChannel > 2)
					return 0;

				const unsigned int offset = inChannel * 7;

				const unsigned short pulse_high = data_source[offset + 3] & 0x0f;
				const unsigned short pulse_low = data_source[offset + 2];

				const unsigned short value = (pulse_high << 8) | pulse_low;

				return value;
			};

			const auto is_channel_filtered = [&data_source](unsigned int inChannel) -> bool
			{
				if (inChannel > 2)
					return false;

				return (data_source[0x17] & (1 << inChannel)) != 0;
			};

			for (unsigned int i = 0; i < 3; ++i)
			{
				if ((*m_Tracks)[i]->IsMuted())
				{
					m_DrawField->DrawBox(color_muted, bar_x, bar_y, bar_width, bar_height);
				}
				else
				{
					DrawPulseWidthBar(bar_x, bar_y, bar_width, bar_height, get_pulse_value(i), is_channel_filtered(i) ? color_bar_filtered_channel : color_bar, color_bar_fill, color_background);
				}
				bar_y += bar_spacing;
			}

			const auto get_filter_value = [&data_source]() -> unsigned short
			{
				const unsigned short filter_high = data_source[0x16];
				const unsigned short filter_low = data_source[0x15] & 7;

				const unsigned short value = (filter_high << 3) | filter_low;

				return value;
			};

			DrawBar(bar_x, bar_y, bar_width, bar_height, get_filter_value(), 0x07ff, color_bar, color_bar_fill_filter);
		}
	}


	void VisualizerComponentPulseFilterState::DrawBar(
		int inX,
		int inY,
		int inWidth,
		int inHeight,
		int inValue,
		int inMaxValue,
		const Foundation::Color& inBarColor,
		const Foundation::Color& inBarColorFill)
	{
		m_DrawField->DrawBox(inBarColor, inX, inY, inWidth, inHeight);

		if (inValue > 0)
		{
			float width_fraction = static_cast<float>(inValue) / static_cast<float>(inMaxValue);
			int width = static_cast<int>(static_cast<float>(inWidth) * (width_fraction < 0 ? 0 : (width_fraction > 1.0f ? 1.0f : width_fraction)));

			m_DrawField->DrawBox(inBarColorFill, inX, inY + 1, width, inHeight - 2);
		}
	}


	// Draw a bar for pulse width
	void VisualizerComponentPulseFilterState::DrawPulseWidthBar(
		int inX,
		int inY,
		int inWidth,
		int inHeight,
		int inValue,
		const Foundation::Color& inBarColor,
		const Foundation::Color& inBarColorFill,
		const Foundation::Color& inDividerColor)
	{
		m_DrawField->DrawBox(inBarColor, inX, inY, inWidth, inHeight);

		int maxValue = 0xfff;
		int middleValue = (maxValue / 2);

		if (inValue > 0)
		{
			if (m_PulseWidthStyle == 1)
			{
				int abs_value = inValue > middleValue ? maxValue - inValue : inValue;
				float width_fraction = static_cast<float>(abs_value) / static_cast<float>(maxValue);
				int width = static_cast<int>(static_cast<float>(inWidth) * (width_fraction < 0 ? 0 : (width_fraction > 1.0f ? 1.0f : width_fraction)));
				int x = inValue > middleValue ? inX + (inWidth - width) : inX;
				m_DrawField->DrawBox(inBarColorFill, x, inY + 1, width, inHeight - 2);
			}
			else
			{
				float width_fraction = static_cast<float>(inValue) / static_cast<float>(maxValue);
				int width = static_cast<int>(static_cast<float>(inWidth) * (width_fraction < 0 ? 0 : (width_fraction > 1.0f ? 1.0f : width_fraction)));
				m_DrawField->DrawBox(inBarColorFill, inX, inY + 1, width, inHeight - 2);
			}
		}

		m_DrawField->DrawVerticalLine(inDividerColor, inX + inWidth / 2, inY, inY + inHeight);
	}
}
