#pragma once
namespace Foundation
{
	class IPlatform;
}

namespace Utility
{
	class ConfigFile;
}

namespace Utility
{
	// Singleton for global configuration
	class Global
	{

	public:
		static Global& instance();
		void deletePlatform();
		Foundation::IPlatform& GetPlatform() const;
		Utility::ConfigFile& GetConfig() const;

	private:
		Global();
		Foundation::IPlatform* m_Platform;
		Utility::ConfigFile* m_Config;
	};

}
