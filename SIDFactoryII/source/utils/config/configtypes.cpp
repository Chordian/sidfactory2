#include "utils/config/configtypes.h"
#include "utils/config/configutils.h"
#include "utils/utilities.h"
#include <assert.h>

namespace Utility
{
	namespace Config
	{
		// ConfigValueCharacter - Format: key_name = 'x' (where x is a character)
		ConfigValueCharacter::ConfigValueCharacter(const std::vector<std::string>& inValues)
		{
			for (const auto& value : inValues)
			{
				assert(IsMyType(value));

				// TODO: Implement conversion
				m_Values.push_back(static_cast<ConfigValueCharacter::DATATYPE>(value[1]));
			}
		}


		ValueType ConfigValueCharacter::GetType() const
		{
			return GetMyType();
		}

		
		size_t ConfigValueCharacter::GetValueCount() const
		{
			return m_Values.size();
		}


		void ConfigValueCharacter::AddValues(const IConfigValue& inValuesToAdd)
		{
			const ConfigValueCharacter& my_type = reinterpret_cast<const ConfigValueCharacter&>(inValuesToAdd);

			for (const auto& value : my_type.GetValues())
				m_Values.push_back(value);
		}


		const ConfigValueCharacter::DATATYPE& ConfigValueCharacter::GetValue(size_t inIndex) const
		{
			assert(inIndex >= 0 && inIndex < m_Values.size());
			return m_Values[inIndex];
		}


		const std::vector<ConfigValueCharacter::DATATYPE>& ConfigValueCharacter::GetValues() const
		{
			return m_Values;
		}


		ValueType ConfigValueCharacter::GetMyType()
		{
			return ValueType::Character;
		}


		bool ConfigValueCharacter::IsMyType(const std::string& inValue)
		{
			const size_t length = inValue.length();

			if (length > 1)
				return inValue[0] == '\'' && inValue[length - 1] == '\'';

			return false;
		}


		// ConfigValueInt 
		ConfigValueInt::ConfigValueInt()
		{
		}


		ConfigValueInt::ConfigValueInt(const std::vector<std::string>& inValues)
		{
			for (const auto& value : inValues)
			{
				assert(IsMyType(value));

				if (value[0] != '0' || value[1] != 'x')
					m_Values.push_back(std::stoi(value));
				else
				{
					int hex_value = 0;

					for (size_t i = 2; i < value.length(); ++i)
					{
						const char v = value[i];
						hex_value <<= 4;

						if (v >= '0' && v <= '9')
							hex_value |= v - '0';
						else
							hex_value |= 0x0a + (v - 'a');
					}

					m_Values.push_back(hex_value);
				}
			}
		}


		ValueType ConfigValueInt::GetType() const
		{
			return GetMyType();
		}


		size_t ConfigValueInt::GetValueCount() const
		{
			return m_Values.size();
		}


		void ConfigValueInt::AddValues(const IConfigValue& inValuesToAdd)
		{
			const ConfigValueInt& my_type = reinterpret_cast<const ConfigValueInt&>(inValuesToAdd);

			for (const auto& value : my_type.GetValues())
				m_Values.push_back(value);
		}


		const ConfigValueInt::DATATYPE& ConfigValueInt::GetValue(size_t inIndex) const
		{
			assert(inIndex >= 0 && inIndex < m_Values.size());
			return m_Values[inIndex];
		}


		const std::vector<ConfigValueInt::DATATYPE>& ConfigValueInt::GetValues() const
		{
			return m_Values;
		}


		ValueType ConfigValueInt::GetMyType()
		{
			return ValueType::Integer;
		}


		bool ConfigValueInt::IsMyType(const std::string& inValue)
		{
			const size_t length = inValue.length();

			if (length == 0)
				return false;
		
			// Check if decimal
			const bool is_decimal = [&]()
			{
				for (size_t i = 0; i < length; ++i)
				{
					if (inValue[i] < '0' || inValue[i] > '9')
						return false;
				}

				return true;
			}();

			if (!is_decimal)
			{
				// Check if hexdecimal
				if (length <= 2)
					return false;

				if (inValue[0] != '0' && inValue[1] != 'x')
					return false;

				for (size_t i = 2; i < length; ++i)
				{
					const char value = inValue[i];
					if ((value < '0' || value > '9') && (value < 'a' || value > 'f'))
						return false;
				}
			}

			return true;
		}


		// ConfigValueString 
		ConfigValueString::ConfigValueString(const std::vector<std::string>& inValues)
		{
			for (const auto& value : inValues)
			{
				assert(IsMyType(value));
				m_Values.push_back(value.substr(1, value.length() - 2));
			}
		}
		

		ValueType ConfigValueString::GetType() const
		{
			return GetMyType();
		}

		
		size_t ConfigValueString::GetValueCount() const
		{
			return m_Values.size();
		}


		void ConfigValueString::AddValues(const IConfigValue& inValuesToAdd)
		{
			const ConfigValueString& my_type = reinterpret_cast<const ConfigValueString&>(inValuesToAdd);

			for (const auto& value : my_type.GetValues())
				m_Values.push_back(value);
		}


		const ConfigValueString::DATATYPE& ConfigValueString::GetValue(size_t inIndex) const
		{
			assert(inIndex >= 0 && inIndex < m_Values.size());
			return m_Values[inIndex];
		}


		const std::vector<ConfigValueString::DATATYPE>& ConfigValueString::GetValues() const
		{
			return m_Values;
		}


		ValueType ConfigValueString::GetMyType()
		{
			return ValueType::String;
		}


		bool ConfigValueString::IsMyType(const std::string& inValue)
		{
			const size_t length = inValue.length();

			if (length > 1)
				return inValue[0] == '\"' && inValue[length - 1] == '\"';

			return false;
		}


		// ConfigValueKey - Format: key_name = @key_value:modifier
		ConfigValueKey::ConfigValueKey(const std::vector<std::string>& inValues)
		{
			for (const auto& value : inValues)
			{
				assert(IsMyType(value));

				std::string value_lower_case = StringToLowerCase(value);
				size_t index = value_lower_case.find(':');

				if (index != std::string::npos)
				{
					std::string key_value = value_lower_case.substr(1, index - 1);
					std::string modifier_value = value_lower_case.substr(index + 1, value_lower_case.size() - index);

					Utility::TrimStringInPlace(key_value);
					Utility::TrimStringInPlace(modifier_value);

					m_Values.push_back(CreateValue(key_value, modifier_value));
				}
				else
					m_Values.push_back(CreateValue(value_lower_case.substr(1, value_lower_case.size()), ""));
			}
		}


		ValueType ConfigValueKey::GetType() const
		{
			return GetMyType();
		}


		size_t ConfigValueKey::GetValueCount() const
		{
			return m_Values.size();
		}


		void ConfigValueKey::AddValues(const IConfigValue& inValuesToAdd)
		{
			const ConfigValueKey& my_type = reinterpret_cast<const ConfigValueKey&>(inValuesToAdd);

			for (const auto& value : my_type.GetValues())
				m_Values.push_back(value);
		}


		const ConfigValueKey::Value& ConfigValueKey::GetValue(size_t inIndex) const
		{
			assert(inIndex >= 0 && inIndex < m_Values.size());
			return m_Values[inIndex];
		}


		const std::vector<ConfigValueKey::DATATYPE>& ConfigValueKey::GetValues() const
		{
			return m_Values;
		}


		ValueType ConfigValueKey::GetMyType()
		{
			return ValueType::Character;
		}


		bool ConfigValueKey::IsMyType(const std::string& inValue)
		{
			const size_t length = inValue.length();

			if (length > 1)
				return inValue[0] == '@';

			return false;
		}


		const ConfigValueKey::Value ConfigValueKey::CreateValue(const std::string& inKeyValue, const std::string& inModifierValue)
		{
			return { Private::FindSDLKeycode(inKeyValue), Private::FindModifier(inModifierValue) };
		}


		// ConfigValueRedirect 
		ConfigValueRedirect::ConfigValueRedirect(const std::vector<std::string>& inValues)
		{
			if (inValues.size() > 0)
			{
				const auto& value = inValues[0];
				
				assert(IsMyType(value));
				m_Values.push_back(value.substr(1, value.length() - 1));
			}
		}


		ValueType ConfigValueRedirect::GetType() const
		{
			return GetMyType();
		}


		size_t ConfigValueRedirect::GetValueCount() const
		{
			return m_Values.size();
		}


		void ConfigValueRedirect::AddValues(const IConfigValue& inValuesToAdd)
		{
			const ConfigValueRedirect& my_type = reinterpret_cast<const ConfigValueRedirect&>(inValuesToAdd);

			for (const auto& value : my_type.GetValues())
				m_Values.push_back(value);
		}


		const ConfigValueRedirect::DATATYPE& ConfigValueRedirect::GetValue(size_t inIndex) const
		{
			assert(inIndex >= 0 && inIndex < m_Values.size());
			return m_Values[inIndex];
		}


		const std::vector<ConfigValueRedirect::DATATYPE>& ConfigValueRedirect::GetValues() const
		{
			return m_Values;
		}


		ValueType ConfigValueRedirect::GetMyType()
		{
			return ValueType::Redirect;
		}

	
		bool ConfigValueRedirect::IsMyType(const std::string& inValue)
		{
			const size_t length = inValue.length();

			if (length > 1)
				return inValue[0] == ':';

			return false;
		}
	}
}