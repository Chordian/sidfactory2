#include "configutils.h"



namespace Utility
{
	namespace Config
	{
		namespace Private
		{
			SDL_Keycode FindSDLKeycode(const std::string& inKeyValue)
			{
				struct KeyNameSDLKeycodePair
				{
					const char* m_KeyName;
					SDL_Keycode m_SDLKeyCode;
				};

				static KeyNameSDLKeycodePair pair[] =
				{
					{ "a", SDLK_a },
					{ "b", SDLK_b },
					{ "c", SDLK_c },
					{ "d", SDLK_d },
					{ "e", SDLK_e },
					{ "f", SDLK_f },
					{ "g", SDLK_g },
					{ "h", SDLK_h },
					{ "i", SDLK_i },
					{ "j", SDLK_j },
					{ "k", SDLK_k },
					{ "l", SDLK_l },
					{ "m", SDLK_m },
					{ "n", SDLK_n },
					{ "o", SDLK_o },
					{ "p", SDLK_p },
					{ "q", SDLK_q },
					{ "r", SDLK_r },
					{ "s", SDLK_s },
					{ "t", SDLK_t },
					{ "u", SDLK_u },
					{ "v", SDLK_v },
					{ "w", SDLK_w },
					{ "x", SDLK_x },
					{ "y", SDLK_y },
					{ "z", SDLK_z },
					{ "0", SDLK_0 },
					{ "1", SDLK_1 },
					{ "2", SDLK_2 },
					{ "3", SDLK_3 },
					{ "4", SDLK_4 },
					{ "5", SDLK_5 },
					{ "6", SDLK_6 },
					{ "7", SDLK_7 },
					{ "8", SDLK_8 },
					{ "9", SDLK_9 },
					{ "f1", SDLK_F1 },
					{ "f2", SDLK_F2 },
					{ "f3", SDLK_F3 },
					{ "f4", SDLK_F4 },
					{ "f5", SDLK_F5 },
					{ "f6", SDLK_F6 },
					{ "f7", SDLK_F7 },
					{ "f8", SDLK_F8 },
					{ "f9", SDLK_F9 },
					{ "f10", SDLK_F10 },
					{ "f11", SDLK_F11 },
					{ "f12", SDLK_F12 },
					{ "return", SDLK_RETURN },
					{ "space", SDLK_SPACE },
					{ "up", SDLK_UP },
					{ "down", SDLK_DOWN },
					{ "left", SDLK_LEFT },
					{ "right", SDLK_RIGHT },
					{ "home", SDLK_HOME },
					{ "end", SDLK_END },
					{ "pageup", SDLK_PAGEUP },
					{ "pagedown", SDLK_PAGEDOWN },
					{ "insert", SDLK_INSERT },
					{ "delete", SDLK_DELETE },
					{ "backspace", SDLK_BACKSPACE },
					{ "escape", SDLK_ESCAPE },
					{ "numplus", SDLK_KP_PLUS },
					{ "numminus", SDLK_KP_MINUS },
					{ "numcomma", SDLK_KP_COMMA },
					{ "minus", SDLK_MINUS },
					{ "plus", SDLK_PLUS },
					{ "half", 189 },

					{ nullptr, 0 }
				};

				for (int i=0; pair[i].m_KeyName != nullptr; ++i)
				{
					if (inKeyValue.compare(pair[i].m_KeyName) == 0)
						return pair[i].m_SDLKeyCode;
				}

				return 0;
			}

			Foundation::Keyboard::Modifier FindModifier(const std::string& inModifierValue)
			{
				if (!inModifierValue.empty())
				{
					unsigned int modifier_value = 0;

					if (inModifierValue.find("shift") != std::string::npos)
						modifier_value |= Foundation::Keyboard::Modifier::Shift;
					if (inModifierValue.find("control") != std::string::npos)
						modifier_value |= Foundation::Keyboard::Modifier::Control;
					if (inModifierValue.find("cmd") != std::string::npos)
						modifier_value |= Foundation::Keyboard::Modifier::Cmd;
					if (inModifierValue.find("alt") != std::string::npos)
						modifier_value |= Foundation::Keyboard::Modifier::Alt;

					return static_cast<Foundation::Keyboard::Modifier>(modifier_value);
				}

				return Foundation::Keyboard::Modifier::None;
			}
		}
	}
}