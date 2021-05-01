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
		Global(Global& inOther) = delete;
		Global(Global&& inOther) = delete;
		Global& operator=(Global&&) = delete;

		// TODO: is this needed?
		void deletePlatform();
		Foundation::IPlatform& GetPlatform() const;
		Utility::ConfigFile& GetConfig() const;

	private:
		Global();
		~Global();
		Foundation::IPlatform* m_Platform;
		Utility::ConfigFile* m_Config;
	};

}
