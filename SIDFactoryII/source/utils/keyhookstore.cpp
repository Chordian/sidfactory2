#include "utils/keyhookstore.h"

namespace Utility
{
	KeyHookStore::KeyHookStore() = default;

	void KeyHookStore::Clear()
	{
		m_HookDefinitions.clear();
	}

	void KeyHookStore::PassBaseDefinitions(const std::vector<KeyHookStore::HookDefinition>& inDefinitionList)
	{
		FOUNDATION_ASSERT(m_HookDefinitions.empty());
		for (const auto& definition : inDefinitionList)
			m_HookDefinitions.insert({ definition.m_HookName, definition.m_HookKey });
	}

	void KeyHookStore::OverrideDefinition(const KeyHookStore::HookDefinition& inDefinition)
	{
		auto it = m_HookDefinitions.find(inDefinition.m_HookName);
		FOUNDATION_ASSERT(it != m_HookDefinitions.end());

		it->second = inDefinition.m_HookKey;
	}


	const std::vector<KeyHookStore::Key>& KeyHookStore::GetKey(const std::string inHookName) const
	{
		auto it = m_HookDefinitions.find(inHookName);
		FOUNDATION_ASSERT(it != m_HookDefinitions.end());

		return it->second;
	}


	std::vector<std::string> KeyHookStore::GetAllHookNames()
	{
		std::vector<std::string> hook_names;
		for (const auto& hook : m_HookDefinitions)
			hook_names.push_back(hook.first);

		return hook_names;
	}

}
