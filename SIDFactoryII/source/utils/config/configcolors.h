#pragma once

#include <vector>
#include <string>

namespace Foundation
{
	class Viewport;
}

namespace Utility
{
	class ConfigFile;

	namespace Config
	{
		void ConfigureColors(const ConfigFile& inConfigFile, Foundation::Viewport& ioViewport);
	}
}