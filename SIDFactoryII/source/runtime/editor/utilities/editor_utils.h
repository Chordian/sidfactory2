#pragma once

#include "SDL_keyboard.h"
#include <string>

namespace Editor
{
	namespace EditorUtils
	{
		int GetNoteValue(SDL_Keycode inKeyCode, int inOctave);
		int GetNoteValue(char inKeyCharacter, int inOctave);
		unsigned char ConvertSingleCharHexValueToValue(char inKeyCharacter);
		char ConvertValueToSingleCharHexValue(unsigned char inValue, bool inUppercase);
		std::string ConvertToHexValue(unsigned char inValue, bool inUppercase);
		std::string ConvertToHexValue(unsigned short inValue, bool inUppercase);
	}
}