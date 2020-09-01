#pragma once

#include "utils/config/configtypes.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <typeinfo>

namespace Foundation
{
	class IPlatform;
}

namespace Utility
{
	class ConfigFile
	{
	public:
		ConfigFile(const Foundation::IPlatform& inPlatform, const std::string& inFilename, const std::vector<std::string>& inValidSectionTags);

		bool IsValid() const;
		const std::vector<std::string>& GetValidSectionTags() const;

		void Reload();

		bool HasKey(const std::string& inKey) const;
		const Config::IConfigValue& GetValue(const std::string& inKey) const;

		template<typename T>
		bool HasKeyOfType(const std::string& inKey) const;

		template<typename T>
		const T& GetValue(const std::string& inKey) const;

	private:
		void Load();

		bool m_IsValid;
		const Foundation::IPlatform& m_Platform;

		std::string m_PathAndFilename;
		std::vector<std::string> m_ValidSectionTags;

		std::map<std::string, std::shared_ptr<Config::IConfigValue>> m_Map;
	};

	template<typename T>
	inline bool ConfigFile::HasKeyOfType(const std::string& inKey) const
	{
		if (HasKey(inKey))
			return GetValue(inKey).GetType() == T::GetMyType();

		return false;
	}

	template<typename T>
	inline const T& ConfigFile::GetValue(const std::string& inKey) const
	{
		const T& value = static_cast<const T&>(GetValue(inKey));
		return value;
	}

	
	// Utility functions
	template<typename T, typename RT = typename T::DATATYPE>
	inline RT GetSingleConfigurationValue(const Utility::ConfigFile& inConfigFile, const std::string& inKey, RT inDefault)
	{
		if (inConfigFile.HasKeyOfType<Config::ConfigValueRedirect>(inKey))
		{
			const Config::ConfigValueRedirect& redirection_value = inConfigFile.GetValue<Config::ConfigValueRedirect>(inKey);
			std::string key = redirection_value.GetValue(0);
			return GetSingleConfigurationValue<T>(inConfigFile, key, inDefault);
		}
		else if (inConfigFile.HasKeyOfType<T>(inKey))
		{
			const T& value = inConfigFile.GetValue<T>(inKey);
			if (value.GetValueCount() > 0)
				return value.GetValue(0);
		}

		return inDefault;
	}


	template<typename T, typename RT = typename T::DATATYPE>
	inline std::vector<RT> GetConfigurationValues(const Utility::ConfigFile& inConfigFile, const std::string& inKey, const std::vector<RT> inDefault)
	{
		if (inConfigFile.HasKeyOfType<Config::ConfigValueRedirect>(inKey))
		{
			const Config::ConfigValueRedirect& redirection_value = inConfigFile.GetValue<Config::ConfigValueRedirect>(inKey);
			std::string key = redirection_value.GetValue(0);
			return GetConfigurationValues<T>(inConfigFile, key, inDefault);
		}
		else if (inConfigFile.HasKeyOfType<T>(inKey))
		{
			const T& value = inConfigFile.GetValue<T>(inKey);
			if (value.GetValueCount() > 0)
				return value.GetValues();
		}

		return inDefault;
	}
}