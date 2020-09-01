#include "keyboard.h"
#include <assert.h>

namespace Foundation
{
	Keyboard::Keyboard()
		: m_Collecting(false)
		, m_LastTick(0)
		, m_RepeatDelay(0.3f)
		, m_RepeatInterval(0.05f)
		, m_ModifierMask(0)
	{

	}


	Keyboard::~Keyboard()
	{

	}

	//------------------------------------------------------------------------------------------------------------------------------

	void Keyboard::SetRepeatDelayAndInterval(float inDelay, float inInterval)
	{
		m_RepeatDelay = inDelay;
		m_RepeatInterval = inInterval;
	}

	//------------------------------------------------------------------------------------------------------------------------------

	void Keyboard::BeginCollect()
	{
		assert(!m_Collecting);

		// Clear list of key presses
		m_KeyEventList.clear();
		m_KeyPressList.clear();
		m_KeyReleaseList.clear();
		m_KeyDownList.clear();
		m_KeyTextList.clear();

		// Enable collection mode
		m_Collecting = true;
	}

	void Keyboard::EndCollect()
	{
		// Process held keys
		unsigned int tick = SDL_GetTicks();

		float delta_time = static_cast<float>(tick - m_LastTick) / 1000.0f;
		m_LastTick = tick;

		for (auto& key : m_KeyTimer)
		{
			key.second -= delta_time;
			if (key.second <= 0.0f)
			{
				key.second += m_RepeatInterval;
				m_KeyEventList.push_back(key.first);
			}

			m_KeyDownList.push_back(key.first);
		}

		// Get caps lock state
		m_CapsLockDown = (SDL_GetModState() & KMOD_CAPS) != 0;

		// End collection mode
		m_Collecting = false;
	}

	//------------------------------------------------------------------------------------------------------------------------------

	void Keyboard::KeyDown(SDL_Keycode inKey)
	{
		assert(m_Collecting);

		if (HandleModifier(inKey, true))
			return;

		if (m_KeyTimer.find(inKey) == m_KeyTimer.end())
		{
			m_KeyTimer[inKey] = m_RepeatDelay;
			m_KeyEventList.push_back(inKey);
			m_KeyPressList.push_back(inKey);
		}
	}

	void Keyboard::KeyUp(SDL_Keycode inKey)
	{
		assert(m_Collecting);

		if (HandleModifier(inKey, false))
			return;

		if (m_KeyTimer.find(inKey) != m_KeyTimer.end())
		{
			m_KeyTimer.erase(inKey);
			m_KeyReleaseList.push_back(inKey);
		}
	}

	void Keyboard::KeyText(char* inText)
	{
		assert(m_Collecting);

		for (int i = 0; i < 32; ++i)
		{
			if (inText[i] == '\0')
				break;

			m_KeyTextList.push_back(inText[i]);
		}
	}

	//------------------------------------------------------------------------------------------------------------------------------

	bool Keyboard::IsModifierEmpty() const
	{
		return m_ModifierMask == 0;
	}

	bool Keyboard::IsModifierDown(unsigned int inModifierMask) const
	{
		return (m_ModifierMask & inModifierMask) != 0;
	}

	bool Keyboard::IsModifierDownExclusive(unsigned int inModifierMask) const
	{
		if ((m_ModifierMask & ~inModifierMask) == 0)
			return IsModifierDown(inModifierMask);

		return false;
	}

	bool Keyboard::IsCapsLockDown() const
	{
		return m_CapsLockDown;
	}


	unsigned int Keyboard::GetModiferMask() const
	{
		return m_ModifierMask;
	}

	const std::vector<SDL_Keycode>& Keyboard::GetKeyEventList() const
	{
		return m_KeyEventList;
	}

	const std::vector<SDL_Keycode>& Keyboard::GetKeyPressedList() const
	{
		return m_KeyPressList;
	}

	const std::vector<SDL_Keycode>& Keyboard::GetKeyReleasedList() const
	{
		return m_KeyReleaseList;
	}

	const std::vector<SDL_Keycode>& Keyboard::GetKeyDownList() const
	{
		return m_KeyDownList;
	}

	const std::vector<char>& Keyboard::GetKeyTextList() const
	{
		return m_KeyTextList;
	}


	//------------------------------------------------------------------------------------------------------------------------------

	bool Keyboard::HandleModifier(SDL_Keycode inKey, bool inKeyDown)
	{
		unsigned int modifier = 0;

		switch (inKey)
		{
		case SDLK_LSHIFT:
			modifier = Keyboard::Shift_Left;
			break;
		case SDLK_RSHIFT:
			modifier = Keyboard::Shift_Right;
			break;
		case SDLK_LCTRL:
			modifier = Keyboard::Control_Left;
			break;
		case SDLK_RCTRL:
			modifier = Keyboard::Control_Right;
			break;
		case SDLK_LALT:
			modifier = Keyboard::Alt_Left;
			break;
		case SDLK_RALT:
			modifier = Keyboard::Alt_Right;
			break;
		case SDLK_LGUI:
			modifier = Keyboard::Cmd_Left;
			break;
		case SDLK_RGUI:
			modifier = Keyboard::Cmd_Right;
			break;
		}

		if (modifier != 0)
		{
			if (inKeyDown)
				m_ModifierMask |= modifier;
			else
				m_ModifierMask &= ~modifier;

			return true;
		}

		return false;
	}
}
