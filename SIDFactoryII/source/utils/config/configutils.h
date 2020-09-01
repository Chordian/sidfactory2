#pragma once

#include "foundation/input/keyboard.h"
#include <SDL_keycode.h>
#include <string>

namespace Utility
{
	namespace Config
	{
		namespace Private
		{
			SDL_Keycode FindSDLKeycode(const std::string& inKeyValue);
			Foundation::Keyboard::Modifier FindModifier(const std::string& inModifierValue);
		}
	}
}