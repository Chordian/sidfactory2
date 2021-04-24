#include "global.h"
#include "configfile.h"
#include "foundation/base/assert.h"
#include "foundation/platform/platform_factory.h"
#include <string>
#include <vector>

using namespace Foundation;

namespace Utility
{

	Global& Global::instance()
	{
		static Global instance;
		return instance;
	}

	// TODO: return const
	IPlatform& Global::GetPlatform() const
	{
		FOUNDATION_ASSERT(m_Platform != nullptr);
		return *m_Platform;
	}
	// TODO: return const
	ConfigFile& Global::GetConfig() const
	{
		FOUNDATION_ASSERT(m_Config != nullptr);
		return *m_Config;
	}

	Global::Global()
		: m_Platform(Foundation::CreatePlatform())
	{

		// Read the config file
		std::vector<std::string> valid_configuration_sections;
		valid_configuration_sections.push_back("default");
		valid_configuration_sections.push_back(m_Platform->GetName());
#ifdef _DEBUG
		valid_configuration_sections.push_back("debug");
#endif //

		std::string config_path = m_Platform->Storage_GetConfigHomePath();

		ConfigFile configFile(*m_Platform, config_path + "config.ini", valid_configuration_sections);

		m_Config = &configFile;
	};

	void Global::deletePlatform()
	{
		delete m_Platform;
		m_Platform = nullptr;
	}

}
