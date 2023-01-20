#pragma once

#include "visualizer_component_base.h"

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
			std::shared_ptr<DataSourceSIDRegistersBufferAfLastDriverUpdate> inDataSource
		);
		virtual ~VisualizerComponentPulseFilterState();

		bool ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse) override;
		void Refresh(const DisplayState& inDisplayState) override;

	private:
		void DrawBar(int inX, int inY, int inWidth, int inHeight, int inValue, int inMaxValue, const Foundation::Color& inBarColor, const Foundation::Color& inBarColorFill);
		std::shared_ptr<DataSourceSIDRegistersBufferAfLastDriverUpdate> m_DataSource;
	};
}