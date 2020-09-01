#pragma once

#include "SDL_keycode.h"
#include "foundation/input/keyboard.h"

namespace Foundation
{
	namespace KeyboardUtils
	{
		bool IsModifierExclusivelyDown(int inModifierMask, int inModifiersToCheckAgainst);
		bool IsAcceptableInputText(char inCharacter);
		bool IsAcceptableInputFilename(char inCharacter);
		char FilterDecimalDigit(const SDL_Keycode inKeyCode);
		char FilterHexDigit(const SDL_Keycode inKeyCode);
		char FilterLetter(const SDL_Keycode inKeyCode);
	}
}