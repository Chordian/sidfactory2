#pragma once

#include "SDL.h"
#include <vector>
#include <map>

namespace Foundation
{
	class Keyboard final
	{
	public:
		enum Modifier : unsigned int
		{
			None = 0,
			Shift_Left = 1 << 0,
			Shift_Right = 1 << 1, 
			Shift = Shift_Left | Shift_Right,
			Control_Left = 1 << 2,
			Control_Right = 1 << 3,
			Control = Control_Left | Control_Right,
			Alt_Left = 1 << 4,
			Alt_Right = 1 << 5,
			Alt = Alt_Left | Alt_Right,
			Cmd_Left = 1 << 6,
			Cmd_Right = 1 << 7,
			Cmd = Cmd_Left | Cmd_Right,
		};

		Keyboard();
		~Keyboard();

		void SetRepeatDelayAndInterval(float inDelay, float inInterval);

		void BeginCollect();
		void EndCollect();

		void KeyDown(SDL_Keycode inKey);
		void KeyUp(SDL_Keycode inKey);
		void KeyText(char* inText);

		bool IsModifierEmpty() const;
		bool IsModifierDown(unsigned int inModifierMask) const;
		bool IsModifierDownExclusive(unsigned int inModifierMask) const;

		bool IsCapsLockDown() const;
		
		unsigned int GetModiferMask() const;

		const std::vector<SDL_Keycode>& GetKeyEventList() const;
		const std::vector<SDL_Keycode>& GetKeyPressedList() const;
		const std::vector<SDL_Keycode>& GetKeyReleasedList() const;
		const std::vector<SDL_Keycode>& GetKeyDownList() const;
		const std::vector<char>& GetKeyTextList() const;

	private:
		bool HandleModifier(SDL_Keycode inKey, bool inKeyDown);

		std::vector<SDL_Keycode> m_KeyEventList;
		std::vector<SDL_Keycode> m_KeyPressList;
		std::vector<SDL_Keycode> m_KeyReleaseList;
		std::vector<SDL_Keycode> m_KeyDownList;
		std::vector<char> m_KeyTextList;

		std::map<SDL_Keycode, float> m_KeyTimer;

		bool m_Collecting;
		unsigned int m_LastTick;

		unsigned int m_ModifierMask;
		bool m_CapsLockDown;

		float m_RepeatDelay;
		float m_RepeatInterval;
	};
}
