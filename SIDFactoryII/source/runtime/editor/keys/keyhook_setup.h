#pragma once

#include "utils/keyhookstore.h"

namespace Utility
{
	class ConfigFile;
}

namespace Editor
{
	class KeyHookSetup
	{
	public:
		KeyHookSetup();

		Utility::KeyHookStore& GetKeyHookStore();

		void Reset();
		void ApplyConfigSettings(const Utility::ConfigFile& inConfigFile);

	private:
		void ConfigureStore();
		void ApplyOverrides();

		Utility::KeyHookStore m_KeyHookStore;
	};
}