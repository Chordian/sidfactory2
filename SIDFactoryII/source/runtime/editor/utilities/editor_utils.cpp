#include "runtime/editor/utilities/editor_utils.h"
#include "foundation/input/keyboard_utils.h"
#include <assert.h>

namespace Editor
{
	namespace EditorUtils
	{
		int GetNoteValue(SDL_Keycode inKeyEvent, int inOctave)
		{
			using namespace Foundation;

			char character = KeyboardUtils::FilterLetter(inKeyEvent);

			if (character == 0)
				character = KeyboardUtils::FilterDecimalDigit(inKeyEvent);

			if (character == 0)
			{
				if (inKeyEvent == SDLK_COMMA)
					character = ',';
				else if (inKeyEvent == SDLK_PERIOD)
					character = '.';
			}

			if (character != 0)
				return EditorUtils::GetNoteValue(character, inOctave);
	
			return -1;
		}

		int GetNoteValue(char inKey, int inOctave)
		{
			int note_value = -1;

			switch (inKey)
			{
			case 'z':
				note_value = 0;
				break;
			case 's':
				note_value = 1;
				break;
			case 'x':
				note_value = 2;
				break;
			case 'd':
				note_value = 3;
				break;
			case 'c':
				note_value = 4;
				break;
			case 'v':
				note_value = 5;
				break;
			case 'g':
				note_value = 6;
				break;
			case 'b':
				note_value = 7;
				break;
			case 'h':
				note_value = 8;
				break;
			case 'n':
				note_value = 9;
				break;
			case 'j':
				note_value = 10;
				break;
			case 'm':
				note_value = 11;
				break;
			case ',':
			case 'q':
				note_value = 12;
				break;
			case 'l':
			case '2':
				note_value = 13;
				break;
			case '.':
			case 'w':
				note_value = 14;
				break;
			case '3':
				note_value = 15;
				break;
			case 'e':
				note_value = 16;
				break;
			case 'r':
				note_value = 17;
				break;
			case '5':
				note_value = 18;
				break;
			case 't':
				note_value = 19;
				break;
			case '6':
				note_value = 20;
				break;
			case 'y':
				note_value = 21;
				break;
			case '7':
				note_value = 22;
				break;
			case 'u':
				note_value = 23;
				break;
			case 'i':
				note_value = 24;
				break;
			case '9':
				note_value = 25;
				break;
			case 'o':
				note_value = 26;
				break;
			case '0':
				note_value = 27;
				break;
			case 'p':
				note_value = 28;
				break;
			}

			if (note_value >= 0)
			{
				note_value += inOctave * 12;

				if (note_value < 0x60)
					return note_value;
			}

			return -1;
		}


		unsigned char ConvertSingleCharHexValueToValue(char inKeyCharacter)
		{
			if (inKeyCharacter >= '0' && inKeyCharacter <= '9')
				return static_cast<unsigned char>(inKeyCharacter - '0');
			if (inKeyCharacter >= 'a' && inKeyCharacter <= 'f')
				return static_cast<unsigned char>(0x0a + inKeyCharacter - 'a');
			if (inKeyCharacter >= 'A' && inKeyCharacter <= 'F')
				return static_cast<unsigned char>(0x0a + inKeyCharacter - 'A');

			assert(true);

			return static_cast<unsigned char>(0);
		};


		char ConvertValueToSingleCharHexValue(unsigned char inValue, bool inUppercase)
		{
			if (inValue < 0x0a)
				return '0' + inValue;
			if (inValue < 0x10)
			{
				if (inUppercase)
					return 'A' + (inValue - 0x0a);
				else
					return 'a' + (inValue - 0x0a);
			}

			assert(true);

			return '?';
		}

		std::string ConvertToHexValue(unsigned char inValue, bool inUppercase)
		{
			char buffer[2];

			auto make_character = [&inUppercase](unsigned char inValue) -> char
			{
				if (inValue > 0x0f)
					return 'x';
				if (inValue < 10)
					return '0' + inValue;

				if (inUppercase)
					return 'A' + inValue - 10;

				return 'a' + inValue - 10;
			};

			buffer[0] = make_character((inValue & 0xf0) >> 4);
			buffer[1] = make_character((inValue & 0x0f) >> 0);

			return std::string(buffer, 2);
		}

		std::string ConvertToHexValue(unsigned short inValue, bool inUppercase)
		{
			char buffer[4];

			auto make_character = [&inUppercase](unsigned char inValue) -> char
			{
				if (inValue > 0x0f)
					return 'x';
				if (inValue < 10)
					return '0' + inValue;

				if (inUppercase)
					return 'A' + inValue - 10;

				return 'a' + inValue - 10;
			};

			buffer[0] = make_character((inValue & 0xf000) >> 12);
			buffer[1] = make_character((inValue & 0x0f00) >> 8);
			buffer[2] = make_character((inValue & 0x00f0) >> 4);
			buffer[3] = make_character((inValue & 0x000f) >> 0);

			return std::string(buffer, 4);
		}
	}
}