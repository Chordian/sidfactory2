#pragma once

#include "SDL_keycode.h"
#include "foundation/input/keyboard_utils.h"
#include "utils/keyhookstore.h"
#include <functional>
#include <vector>
#include <string>

namespace Utility
{
	template<typename CONTEXT>
	class KeyHook
	{
	public:
		KeyHook();
		KeyHook(const std::string& inIdentifier, const KeyHookStore& inKeyHookStore, std::function<CONTEXT> inKeyDownCallback);
		KeyHook(const std::string& inIdentifier, const KeyHookStore::Key& inKey, std::function<CONTEXT> inKeyDownCallback);

		const bool operator == (KeyHook<CONTEXT>& inOther) const;
		const bool operator != (KeyHook<CONTEXT>& inOther) const;

		operator bool() const;

		const std::string& GetIdentifier() const;

		bool TryConsume(SDL_Keycode inKeyCode, unsigned int inModifier) const;

		template<typename ...ARGS>
		bool TryConsume(SDL_Keycode inKeyCode, unsigned int inModifier, ARGS... inArgs) const;

	private:
		bool RequireShiftDown(int iKeyIndex) const;
		bool RequireControlDown(int iKeyIndex) const;
		bool RequireAltDown(int iKeyIndex) const;

		template<typename ...ARGS>
		bool OnConsume(ARGS... inArgs) const;

		std::string m_Identifier;
		std::vector<KeyHookStore::Key> m_Keys;

		std::function<CONTEXT> m_KeyDownCallback;
	};


	template<typename CONTEXT>
	inline bool ConsumeInputKeyHooks(SDL_Keycode inKeyCode, unsigned int inModifier, const std::vector<KeyHook<CONTEXT>>& inKeyHookList)
	{
		for (const auto& key_hook : inKeyHookList)
		{
			if (key_hook.TryConsume(inKeyCode, inModifier))
				return true;
		}
		return false;
	}


	template<typename CONTEXT, typename ...ARGS>
	inline bool ConsumeInputKeyHooks(SDL_Keycode inKeyCode, unsigned int inModifier, const std::vector<KeyHook<CONTEXT>>& inKeyHookList, ARGS... inArgs)
	{
		for (const auto& key_hook : inKeyHookList)
		{
			if (key_hook.TryConsume(inKeyCode, inModifier, inArgs...))
				return true;
		}
		return false;
	}


	template<typename CONTEXT>
	KeyHook<CONTEXT>::KeyHook()
	{
	}


	template<typename CONTEXT>
	KeyHook<CONTEXT>::KeyHook(const std::string& inIdentifier, const KeyHookStore& inKeyHookStore, std::function<CONTEXT> inKeyDownCallback)
		: m_Identifier(inIdentifier)
		, m_KeyDownCallback(inKeyDownCallback)
	{
		m_Keys = inKeyHookStore.GetKey(inIdentifier);

		//m_KeyCode = key.m_KeyCode;
		//m_Modifiers = key.m_Modifiers;
	}


	template<typename CONTEXT>
	KeyHook<CONTEXT>::KeyHook(const std::string& inIdentifier, const KeyHookStore::Key& inKey, std::function<CONTEXT> inKeyDownCallback)
		: m_Identifier(inIdentifier)
		, m_KeyDownCallback(inKeyDownCallback)
	{
		m_Keys.push_back(inKey);
	}


	template<typename CONTEXT>
	const bool KeyHook<CONTEXT>::operator == (KeyHook<CONTEXT>& inOther) const
	{
		if (m_Keys.size() != inOther.m_Keys.size())
			return false;

		const size_t key_count = m_Keys.size();

		for (int i = 0; i < key_count; ++i)
		{
			if (m_Keys[i].m_KeyCode != inOther.m_Keys[i].m_KeyCode)
				return false;

			if (m_Keys[i].m_Modifiers != 0 || inOther.m_Keys[i].m_Modifiers != 0)
			{
				if (RequireShiftDown(i) != inOther.RequireShiftDown(i))
					return false;
				if (RequireControlDown(i) != inOther.RequireControlDown(i))
					return false;
				if (RequireAltDown(i) != inOther.RequireAltDown(i))
					return false;
			}
		}

		return true;
	}

	template<typename CONTEXT>
	const bool KeyHook<CONTEXT>::operator != (KeyHook<CONTEXT>& inOther) const
	{
		return !(*this == inOther);
	}

	template<typename CONTEXT>
	KeyHook<CONTEXT>::operator bool() const
	{
		return m_Keys.size() > 0;
	}

	template<typename CONTEXT>
	const std::string& KeyHook<CONTEXT>::GetIdentifier() const
	{
		return m_Identifier;
	}



	template<typename CONTEXT>
	bool KeyHook<CONTEXT>::RequireShiftDown(int inKeyIndex) const
	{
		FOUNDATION_ASSERT(inKeyIndex < static_cast<int>(m_Keys.size()));
		return (m_Keys[inKeyIndex].m_Modifiers & Foundation::Keyboard::Shift) != 0;
	}

	template<typename CONTEXT>
	bool KeyHook<CONTEXT>::RequireControlDown(int inKeyIndex) const
	{
		FOUNDATION_ASSERT(inKeyIndex < static_cast<int>(m_Keys.size()));
		return (m_Keys[inKeyIndex].m_Modifiers & Foundation::Keyboard::Control) != 0;
	}

	template<typename CONTEXT>
	bool KeyHook<CONTEXT>::RequireAltDown(int inKeyIndex) const
	{
		FOUNDATION_ASSERT(inKeyIndex < static_cast<int>(m_Keys.size()));
		return (m_Keys[inKeyIndex].m_Modifiers & Foundation::Keyboard::Alt) != 0;
	}


	template<typename CONTEXT>
	bool KeyHook<CONTEXT>::TryConsume(SDL_Keycode inKeyCode, unsigned int inModifier) const
	{
		for (const auto& key : m_Keys)
		{
			if (key.m_KeyCode == inKeyCode)
			{
				if (Foundation::KeyboardUtils::IsModifierExclusivelyDown(inModifier, key.m_Modifiers))
					return OnConsume();
			}
		}

		return false;
	}


	template<typename CONTEXT>
	template<typename ...ARGS>
	bool KeyHook<CONTEXT>::TryConsume(SDL_Keycode inKeyCode, unsigned int inModifier, ARGS... inArgs) const
	{
		for (const auto& key : m_Keys)
		{
			if (key.m_KeyCode == inKeyCode)
			{
				if (Foundation::KeyboardUtils::IsModifierExclusivelyDown(inModifier, key.m_Modifiers))
					return OnConsume(inArgs...);
			}
		}

		return false;
	}


	template<typename CONTEXT>
	template<typename ...ARGS>
	bool KeyHook<CONTEXT>::OnConsume(ARGS... inArgs) const
	{
		if (m_KeyDownCallback)
			return m_KeyDownCallback(inArgs...);

		return false;
	}

}
