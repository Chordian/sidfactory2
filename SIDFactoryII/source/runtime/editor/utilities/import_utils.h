#pragma once

#include "runtime/editor/screens/screen_base.h"
#include "runtime/editor/driver/driver_info.h"
#include "runtime/emulation/cpumemory.h"
#include "utils/c64file.h"

namespace Editor
{
	namespace ImportUtils
	{
		void Import(
			ScreenBase* inCallerScreen,
			DriverInfo& inLoadedDriverInfo,
			Emulation::CPUMemory& inCPUMemory,
			const DriverInfo& inImportDriverInfo,
			const Utility::C64File& inImportFile
		);
	}
}