#pragma once

#include "foundation/graphics/types.h"
#include <memory>
#include <vector>

namespace Foundation
{
	class Viewport;
	class TextField;
	class Keyboard;
	class Mouse;
}

namespace Emulation
{
	class CPUMemory;
}

namespace Utility
{
	template<typename CONTEXT>
	class KeyHook;
}

namespace Editor
{
	class DriverInfo;
	class ComponentsManager;
	class ComponentMemoryView;

	class DebugViews final
	{
	public:
		DebugViews(Foundation::Viewport* inViewport, ComponentsManager* inComponentsManager, Emulation::CPUMemory* inCPUMemory, const Foundation::Extent& inMainTextFieldDimensions, std::shared_ptr<const DriverInfo> inDriverInfo);
		~DebugViews();

		void SetEnabled(bool inEnabled);
		bool IsEnabled() const;

		void SetMemoryAddress(unsigned short inMemoryAddress);
		void SetEventPosition(int inEventPos);

		void Update(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse);

	private:
		void CreateViews(ComponentsManager* inComponentsManager);

		bool m_Enabled;

		unsigned short m_MemoryAddress;
		int m_EventPos;

		std::shared_ptr<const DriverInfo> m_DriverInfo;

		Emulation::CPUMemory* m_CPUMemory;
		Foundation::Viewport* m_Viewport;
		Foundation::TextField* m_TextField;

		ComponentsManager* m_ComponentsManager;

		std::shared_ptr<ComponentMemoryView> m_ComponentMemoryView;
		//std::vector<Utility::KeyHook<bool(void)>> m_KeyHookTests;
		std::vector<int> m_KeyHookTestValues;

		static const int ComponentBaseID;
		static const int ComponentGroupID;
	};
}