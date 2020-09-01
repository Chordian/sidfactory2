#include "keyboard_utils.h"
#include "keyboard.h"

namespace Foundation
{
	namespace KeyboardUtils
	{
		bool IsModifierExclusivelyDown(int inModifierMask, int inModifiersToCheckAgainst)
		{
			if ((inModifierMask & ~inModifiersToCheckAgainst) != 0)
				return false;
			if (static_cast<bool>(inModifiersToCheckAgainst & Keyboard::Shift) != static_cast<bool>(inModifierMask & Keyboard::Shift))
				return false;
			if (static_cast<bool>(inModifiersToCheckAgainst & Keyboard::Control) != static_cast<bool>(inModifierMask & Keyboard::Control))
				return false;
			if (static_cast<bool>(inModifiersToCheckAgainst & Keyboard::Alt) != static_cast<bool>(inModifierMask & Keyboard::Alt))
				return false;

			return true;
		}


		bool IsAcceptableInputText(char inCharacter)
		{
			static char acceptable_characters[] = " 0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.,-+_!\"#&%()[]//\\";

			for (int i = 0; i < sizeof(acceptable_characters); ++i)
			{
				if (acceptable_characters[i] == inCharacter)
					return true;
			}

			return false;
		}


		bool IsAcceptableInputFilename(char inCharacter)
		{
			static char acceptable_characters[] = " 0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.,-+_!()[]";

			for (int i = 0; i < sizeof(acceptable_characters); ++i)
			{
				if (acceptable_characters[i] == inCharacter)
					return true;
			}

			return false;
		}


		char FilterDecimalDigit(const SDL_Keycode inKeyCode)
		{
			if (inKeyCode >= SDLK_0 && inKeyCode <= SDLK_9)
				return '0' + static_cast<char>(inKeyCode - SDLK_0);
			if (inKeyCode >= SDLK_KP_1 && inKeyCode <= SDLK_KP_9)
				return '1' + static_cast<char>(inKeyCode - SDLK_KP_1);
			if (inKeyCode == SDLK_KP_0)
				return '0';

			return 0;
		}


		char FilterHexDigit(const SDL_Keycode inKeyCode)
		{
			if (inKeyCode >= SDLK_a && inKeyCode <= SDLK_f)
				return 'a' + static_cast<char>(inKeyCode - SDLK_a);
			return FilterDecimalDigit(inKeyCode);
		}


		char FilterLetter(const SDL_Keycode inKeyCode)
		{
			if (inKeyCode >= SDLK_a && inKeyCode <= SDLK_z)
				return 'a' + static_cast<char>(inKeyCode - SDLK_a);

			return 0;
		}
	}
}