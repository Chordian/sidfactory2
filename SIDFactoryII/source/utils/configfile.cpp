#include "utils/configfile.h"
#include "utils/config/configtypes.h"
#include "utils/utilities.h"
#include "libraries/ghc/fs_std.h"
#include "foundation/platform/iplatform.h"
#include "foundation/base/assert.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory>
#ifndef _SF2_WINDOWS
#include <wordexp.h>
#endif

using namespace fs;
using namespace Foundation;


namespace Utility
{
	namespace Details
	{
		struct ConfigKeyValuePair
		{
			bool m_Add;
			std::string m_Key;
			std::vector<std::string> m_Values;
		};

		void GatherLines(const IPlatform& inPlatform, const char* inData, const unsigned int inSize, const std::vector<std::string>& inSectionTags, std::vector<std::string>& outLineList, bool& ioInValidSection);
		bool HandleSectionChanges(const std::string& inLine, const std::vector<std::string>& inSectionTags, bool& inIsInValidSection);
		bool HandleCommandTags(const IPlatform& inPlatform, const std::string& inLine, const std::vector<std::string>& inSectionTags, std::vector<std::string>& outLineList, bool& ioInValidSection);
		void StripComments(std::vector<std::string>& ioLines);
		void GatherKeyValuePairs(const std::vector<std::string>& inLineList, std::vector<ConfigKeyValuePair>& outKeyValuePairs);
		void GatherKeyValues(const std::string& inValue, std::vector<std::string>& outValues);

		void DoInclude(const IPlatform& inPlatform, const path& inIncludeFile, const std::vector<std::string>& inSectionTags, std::vector<std::string>& outLineList, bool& ioInValidSection);
		
		std::shared_ptr<Utility::Config::IConfigValue> BuildConfigValue(const std::vector<std::string>& inValues);

		template<class TYPE>
		std::shared_ptr<Utility::Config::IConfigValue> TryBuildConfigValue(const std::vector<std::string>& inValues);

		void TryAddValue(std::shared_ptr<Utility::Config::IConfigValue>& inExistingValue, std::shared_ptr<Utility::Config::IConfigValue>& inValueToAdd);
	}

	ConfigFile::ConfigFile(const IPlatform& inPlatform, const std::string& inFilename, const std::vector<std::string>& inValidSectionTags)
		: m_ValidSectionTags(inValidSectionTags)
		, m_Platform(inPlatform)
		, m_IsValid(false)
	{
		path path_and_filename = inFilename;
		m_PathAndFilename = path_and_filename.string();

		Load();
	}


	bool ConfigFile::IsValid() const
	{
		return m_IsValid;
	}


	const std::vector<std::string>& ConfigFile::GetValidSectionTags() const
	{
		return m_ValidSectionTags;
	}



	void ConfigFile::Reload()
	{
		m_Map.clear();
		Load();
	}


	bool ConfigFile::HasKey(const std::string& inKey) const
	{
		return m_Map.find(inKey) != m_Map.end();
	}


	const Config::IConfigValue& ConfigFile::GetValue(const std::string& inKey) const
	{
		FOUNDATION_ASSERT(HasKey(inKey));
		return *(m_Map.find(inKey)->second);
	}


	void ConfigFile::Load()
	{
		using namespace Details;

		void* data = nullptr;
		long file_size = 0;

		if (Utility::ReadFile(m_PathAndFilename, 0, &data, file_size))
		{
			std::vector<std::string> line_list;
			bool in_valid_section = true;
			GatherLines(m_Platform, static_cast<char*>(data), static_cast<unsigned int>(file_size), m_ValidSectionTags, line_list, in_valid_section);

			// Delete file data
			delete[] static_cast<char*>(data);

			// Strip comments from the lines
			StripComments(line_list);

			// Build key value pairs
			std::vector<ConfigKeyValuePair> key_value_pairs;
			GatherKeyValuePairs(line_list, key_value_pairs);

			// Build config map
			for (const auto& key_value_pair : key_value_pairs)
			{
				std::shared_ptr<Config::IConfigValue> value = BuildConfigValue(key_value_pair.m_Values);

				if (value != nullptr)
				{
					if (key_value_pair.m_Add && m_Map.find(key_value_pair.m_Key) != m_Map.end())
						TryAddValue(m_Map[key_value_pair.m_Key], value);
					else
						m_Map[key_value_pair.m_Key] = value;
				}
			}

			m_IsValid = true;

			return;
		}

		m_IsValid = false;
	}



	namespace Details
	{
		void GatherLines(const IPlatform& inPlatform, const char* inData, const unsigned int inSize, const std::vector<std::string>& inSectionTags, std::vector<std::string>& outLineList, bool& ioInValidSection)
		{
			bool valid = false;
			unsigned int begin = 0;

			for (unsigned int i = 0; i < inSize; ++i)
			{
				const char value = inData[i];

				if ((value >= 0x20 && value <= 0x7e) || value == 0x09)
				{
					if (!valid)
					{
						valid = true;
						begin = i;
					}
				}
				else
				{
					if (valid)
					{
						valid = false;

						std::string line = std::string(inData + begin, i - begin);

						if (ioInValidSection && HandleCommandTags(inPlatform, line, inSectionTags, outLineList, ioInValidSection))
							continue;
						if (HandleSectionChanges(line, inSectionTags, ioInValidSection))
							continue;
						
						if(ioInValidSection)
							outLineList.push_back(line);
					}
				}
			}

			if (valid)
			{
				valid = false;

				std::string line = std::string(inData + begin, inSize - begin);

				if (ioInValidSection && HandleCommandTags(inPlatform, line, inSectionTags, outLineList, ioInValidSection))
					return;
				if (HandleSectionChanges(line, inSectionTags, ioInValidSection))
					return;

				if(ioInValidSection)
					outLineList.push_back(line);
			}
		}


		bool HandleSectionChanges(const std::string& inLine, const std::vector<std::string>& inSectionTags, bool& ioIsInValidSection)
		{
			std::string trimmed_line = Utility::TrimString(inLine);

			if (trimmed_line.size() > 0 && trimmed_line[0] == '[')
			{
				size_t end = trimmed_line.find(']');

				if (end != std::string::npos)
				{
					std::string section_tag = inLine.substr(1, end - 1);
					ioIsInValidSection = std::find(inSectionTags.begin(), inSectionTags.end(), section_tag) != inSectionTags.end();
				}

				// Consume the line
				return true;
			}

			return false;
		}


		bool HandleCommandTags(const IPlatform& inPlatform, const std::string& inLine, const std::vector<std::string>& inSectionTags, std::vector<std::string>& outLineList, bool& ioInValidSection)
		{
			std::string trimmed_line = Utility::TrimString(inLine);
			std::string trimmed_line_lowercase = Utility::StringToLowerCase(trimmed_line);

			if (trimmed_line_lowercase.size() > 0 && trimmed_line_lowercase[0] == '#')
			{
				if (trimmed_line_lowercase.find("include") == 1)
				{
					size_t begin = trimmed_line.find_first_of('\"');
					size_t end = trimmed_line.find_last_of('\"');

					if (begin != std::string::npos && end != std::string::npos && begin + 1 < end)
					{
						trimmed_line = trimmed_line.substr(begin + 1, end - begin - 1);
                        path filename = inPlatform.OS_ParsePath(trimmed_line);
						
						if (is_regular_file(filename))
							DoInclude(inPlatform, filename, inSectionTags, outLineList, ioInValidSection);
					}
				}

				// Consume the line
				return true;
			}

			return false;
		}


		void DoInclude(const IPlatform& inPlatform, const path& inIncludeFile, const std::vector<std::string>& inSectionTags, std::vector<std::string>& outLineList, bool& ioInValidSection)
		{
			void* data = nullptr;
			long file_size = 0;

			if (Utility::ReadFile(inIncludeFile.string(), 0, &data, file_size))
			{
				GatherLines(inPlatform, static_cast<char*>(data), static_cast<unsigned int>(file_size), inSectionTags, outLineList, ioInValidSection);

				// Delete file data
				delete[] static_cast<char*>(data);
			}
		}


		void StripComments(std::vector<std::string>& ioLines)
		{
			for (std::string& line : ioLines)
			{
				size_t comment_index = line.find("//");
				if (comment_index != std::string::npos)
					line = line.substr(0, comment_index);
			}
		}



		void GatherKeyValuePairs(const std::vector<std::string>& inLineList, std::vector<ConfigKeyValuePair>& outKeyValuePairs)
		{
			for (const auto& line : inLineList)
			{
				bool add = true;
				size_t index = line.find("+=");

				if (index == std::string::npos)
				{
					add = false;
					index = line.find('=');
				}

				if (index != std::string::npos)
				{
					size_t offset = add ? 2 : 1;

					std::string key = line.substr(0, index);
					std::string value = line.substr(index + offset, line.length() - (index + offset));

					Utility::TrimStringInPlace(key);

					std::vector<std::string> values;
					GatherKeyValues(value, values);

					outKeyValuePairs.push_back({ add, key, values });
				}
			}
		}


		void GatherKeyValues(const std::string& inValue, std::vector<std::string>& outValues)
		{
			const size_t length = inValue.length();
			unsigned int begin = 0;
			bool inside_quotation = false;
			
			for (size_t i = 0; i < length; ++i)
			{
				const char character = inValue[i];

				if (character == '\"')
					inside_quotation = !inside_quotation;

				if (character == ',' && !inside_quotation)
				{
					std::string value = inValue.substr(begin, i - begin);
					begin = i + 1;

					Utility::TrimStringInPlace(value);

					outValues.push_back(value);
				}
			}

			if (begin < length)
			{
				std::string value = inValue.substr(begin, length - begin);
				Utility::TrimStringInPlace(value);

				outValues.push_back(value);
			}
		}


		std::shared_ptr<Utility::Config::IConfigValue> BuildConfigValue(const std::vector<std::string>& inValues)
		{
			std::shared_ptr<Utility::Config::IConfigValue> config_value;

			config_value = TryBuildConfigValue<Utility::Config::ConfigValueCharacter>(inValues);
			if (config_value != nullptr)
				return config_value;
			config_value = TryBuildConfigValue<Utility::Config::ConfigValueInt>(inValues);
			if (config_value != nullptr)
				return config_value;
			config_value = TryBuildConfigValue<Utility::Config::ConfigValueFloat>(inValues);
			if (config_value != nullptr)
				return config_value;
			config_value = TryBuildConfigValue<Utility::Config::ConfigValueString>(inValues);
			if (config_value != nullptr)
				return config_value;
			config_value = TryBuildConfigValue<Utility::Config::ConfigValueKey>(inValues);
			if (config_value != nullptr)
				return config_value;
			config_value = TryBuildConfigValue<Utility::Config::ConfigValueRedirect>(inValues);
			if (config_value != nullptr)
				return config_value;
			return nullptr;
		}


		template<class TYPE>
		std::shared_ptr<Utility::Config::IConfigValue> TryBuildConfigValue(const std::vector<std::string>& inValues)
		{
			for (const auto& value : inValues)
			{
				if (!TYPE::IsMyType(value))
					return nullptr;
			}

			return std::make_shared<TYPE>(inValues);
		}


		void TryAddValue(std::shared_ptr<Utility::Config::IConfigValue>& inExistingValue, std::shared_ptr<Utility::Config::IConfigValue>& inValueToAdd)
		{
			if (inExistingValue->GetType() == inValueToAdd->GetType())
				inExistingValue->AddValues(*inValueToAdd);
		}
	}
}
