#pragma once

#include "foundation/base/assert.h"
#include <functional>
#include <vector>
#include <string>
#include <map>
#include "SDL_keycode.h"

namespace Utility
{
	class KeyHookStore
	{
	public:
		struct Key
		{
			SDL_Keycode m_KeyCode;
			unsigned int m_Modifiers;
		};

		struct HookDefinition
		{
			std::string m_HookName;
			std::vector<Key> m_HookKey;
		};

		KeyHookStore();

		void Clear();

		void PassBaseDefinitions(const std::vector<HookDefinition>& inDefinitionList);
		void OverrideDefinition(const HookDefinition& inDefinition);

		const std::vector<Key>& GetKey(const std::string inHookName) const;

		std::vector<std::string> GetAllHookNames();

	private:
		std::map<std::string, std::vector<Key>> m_HookDefinitions;
	};
}