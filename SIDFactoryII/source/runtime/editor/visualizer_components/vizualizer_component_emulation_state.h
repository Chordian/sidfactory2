#pragma once

#include "visualizer_component_base.h"
#include <memory>
#include <functional>

namespace Foundation
{
	enum class Color : unsigned short;
}

namespace Editor
{
	class DataSourceFlightRecorder;

	class VisualizerComponentEmulationState : public VisualizerComponentBase
	{
	public:
		VisualizerComponentEmulationState(
			int inID, 
			Foundation::DrawField* inDrawField, 
			int inX, 
			int inY, 
			int inWidth, 
			int inHeight, 
			std::shared_ptr<DataSourceFlightRecorder> inDataSource
		);
		virtual ~VisualizerComponentEmulationState();

		bool ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse) override;
		void Refresh(const DisplayState& inDisplayState) override;

	private:
		void DrawColoredFilled(
			const std::function<const int(unsigned int)>& inValueFunction,	
			const std::function<Foundation::Color(const int)>& inColorFunction
		);

		std::shared_ptr<DataSourceFlightRecorder> m_DataSource;
	};
}