#pragma once

#include "foundation/graphics/types.h"
#include <memory>
#include <vector>

namespace Foundation
{
	class Viewport;
	class TextField;
	class DrawField;
	class Keyboard;
	class Mouse;
}

namespace Emulation
{
	class CPUMemory;
	class ExecutionHandler;
}

namespace Editor
{
	class DriverInfo;
	class ComponentsManager;
	class ComponentFlightRecorderView;
	class VisualizerComponentEmulationState;

	class OverlayFlightRecorder
	{
	public:
		OverlayFlightRecorder(Foundation::Viewport* inViewport, ComponentsManager* inComponentsManager, Emulation::CPUMemory* inCPUMemory, Emulation::ExecutionHandler* inExecutionHandler, const Foundation::Extent& inMainTextFieldDimensions);
		~OverlayFlightRecorder();

		void SetEnabled(bool inEnabled);
		bool IsEnabled() const;

	private:
		void AddComponents();

		bool m_Enabled;

		Emulation::ExecutionHandler* m_ExecutionHandler;
		Emulation::CPUMemory* m_CPUMemory;
		Foundation::Viewport* m_Viewport;
		Foundation::TextField* m_TextField;
		Foundation::DrawField* m_DrawField;

		ComponentsManager* m_ComponentsManager;

		std::shared_ptr<VisualizerComponentEmulationState> m_VisualizerEmulationState;

		static const int ComponentBaseID;
		static const int ComponentGroupID;
	};
}