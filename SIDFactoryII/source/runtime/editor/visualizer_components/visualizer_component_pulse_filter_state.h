#pragma once

#include "visualizer_component_base.h"
#include <memory>
#include "runtime/editor/datasources/datasource_track_components.h"

namespace Foundation
{
	enum class Color : unsigned short;
}

namespace Editor
{
	class DataSourceSIDRegistersBufferAfLastDriverUpdate;

	class VisualizerComponentPulseFilterState : public VisualizerComponentBase
	{
	public:
		VisualizerComponentPulseFilterState(
			int inID,
			Foundation::DrawField* inDrawField,
			int inX,
			int inY,
			int inWidth,
			int inHeight,
			std::shared_ptr<DataSourceSIDRegistersBufferAfLastDriverUpdate> inDataSource,
			std::shared_ptr<DataSourceTrackComponents> inTracks
		);
		virtual ~VisualizerComponentPulseFilterState();

		bool ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse) override;
		void Refresh(const DisplayState& inDisplayState) override;

	private:
		void DrawBar(
			int inX,
			int inY,
			int inWidth,
			int inHeight,
			 int inValue,
			 int inMaxValue,
			 const Foundation::Color& inBarColor,
			 const Foundation::Color& inBarColorFill);

		void DrawBarWithCenterDivider(
			int inX,
			int inY,
			int inWidth,
			int inHeight,
			int inValue,
			int inMaxValue,
			const Foundation::Color& inBarColor,
			const Foundation::Color& inBarColorFill,
			const Foundation::Color& inDividerColor);

		std::shared_ptr<DataSourceSIDRegistersBufferAfLastDriverUpdate> m_DataSource;
		std::shared_ptr<DataSourceTrackComponents> m_Tracks;

		int m_PulseWidthStyle;
	};
}