#pragma once

#include <memory>
#include "runtime/editor/driver/driver_info.h"
#include "runtime/emulation/cpumemory.h"

namespace Editor
{
	struct ZeroPageRange
	{
		unsigned char m_LowestZeroPage;
		unsigned char m_HighestZeroPage;
	};

	ZeroPageRange GetZeroPageRangeFromDriver(Emulation::CPUMemory& inCPUMemory, const DriverInfo& inDriverInfo);
}