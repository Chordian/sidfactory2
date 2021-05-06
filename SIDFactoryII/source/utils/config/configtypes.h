#pragma once

#include <SDL_keycode.h>
#include <vector>
#include <string>

namespace Utility
{
	namespace Config
	{
		enum class ValueType : unsigned int
		{
			Key,
			Character,
			Integer,
			Float,
			String,
			Redirect
		};

		class IConfigValue
		{
		public:
			virtual ~IConfigValue() { }
			virtual ValueType GetType() const = 0;
			virtual size_t GetValueCount() const = 0;
			virtual void AddValues(const IConfigValue& inValuesToAdd) = 0;
		};


		class ConfigValueCharacter : public IConfigValue
		{
		public:
			using DATATYPE = char;

			ConfigValueCharacter(const std::vector<std::string>& inValues);
			ValueType GetType() const override;
			size_t GetValueCount() const override;
			void AddValues(const IConfigValue& inValuesToAdd) override;
			const DATATYPE& GetValue(size_t inIndex) const;
			const std::vector<DATATYPE>& GetValues() const;

			static ValueType GetMyType();
			static bool IsMyType(const std::string& inValue);

		private:
			std::vector<DATATYPE> m_Values;
		};


		class ConfigValueInt : public IConfigValue
		{
		public:
			using DATATYPE = int;

			ConfigValueInt();
			ConfigValueInt(const std::vector<std::string>& inValues);
			ValueType GetType() const override;
			size_t GetValueCount() const override;
			void AddValues(const IConfigValue& inValuesToAdd) override;
			const DATATYPE& GetValue(size_t inIndex) const;
			const std::vector<DATATYPE>& GetValues() const;

			static ValueType GetMyType();
			static bool IsMyType(const std::string& inValue);

		private:
			std::vector<DATATYPE> m_Values;
		};


		class ConfigValueFloat : public IConfigValue
		{
		public:
			using DATATYPE = float;

			ConfigValueFloat();
			ConfigValueFloat(const std::vector<std::string>& inValues);
			ValueType GetType() const override;
			size_t GetValueCount() const override;
			void AddValues(const IConfigValue& inValuesToAdd) override;
			const DATATYPE& GetValue(size_t inIndex) const;
			const std::vector<DATATYPE>& GetValues() const;

			static ValueType GetMyType();
			static bool IsMyType(const std::string& inValue);

		private:
			std::vector<DATATYPE> m_Values;
		};


		class ConfigValueString : public IConfigValue
		{
		public:
			using DATATYPE = std::string;

			ConfigValueString(const std::vector<std::string>& inValues);
			ValueType GetType() const override;
			size_t GetValueCount() const override;
			void AddValues(const IConfigValue& inValuesToAdd) override;
			const DATATYPE& GetValue(size_t inIndex) const;
			const std::vector<DATATYPE>& GetValues() const;

			static ValueType GetMyType();
			static bool IsMyType(const std::string& inValue);

		private:
			std::vector<DATATYPE> m_Values;
		};


		class ConfigValueKey : public IConfigValue
		{
		public:
			struct Value
			{
				SDL_Keycode m_Key;
				unsigned int m_Modifier;
			};

			using DATATYPE = Value;

			ConfigValueKey(const std::vector<std::string>& inValues);
			ValueType GetType() const override;
			size_t GetValueCount() const override;
			void AddValues(const IConfigValue& inValuesToAdd) override;
			const DATATYPE& GetValue(size_t inIndex) const;
			const std::vector<DATATYPE>& GetValues() const;

			static ValueType GetMyType();
			static bool IsMyType(const std::string& inValue);

		private:
			const Value CreateValue(const std::string& inKeyValue, const std::string& inModifierValue);

			std::vector<DATATYPE> m_Values;
		};


		class ConfigValueRedirect : public IConfigValue
		{
		public:
			using DATATYPE = std::string;

			ConfigValueRedirect(const std::vector<std::string>& inValues);
			ValueType GetType() const override;
			size_t GetValueCount() const override;
			void AddValues(const IConfigValue& inValuesToAdd) override;
			const DATATYPE& GetValue(size_t inIndex) const;
			const std::vector<DATATYPE>& GetValues() const;

			static ValueType GetMyType();
			static bool IsMyType(const std::string& inValue);

		private:
			std::vector<DATATYPE> m_Values;
		};
	}
}