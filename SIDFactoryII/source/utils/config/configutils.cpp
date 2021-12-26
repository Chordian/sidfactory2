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

				static KeyNameSDLKeycodePair pair[] = {

					// taken from SDL_keycode.h

					{ "return", SDLK_RETURN },
					{ "escape", SDLK_ESCAPE },
					{ "backspace", SDLK_BACKSPACE },
					{ "tab", SDLK_TAB },
					{ "space", SDLK_SPACE },
					{ "exclaim", SDLK_EXCLAIM },
					{ "quotedbl", SDLK_QUOTEDBL },
					{ "hash", SDLK_HASH },
					{ "percent", SDLK_PERCENT },
					{ "dollar", SDLK_DOLLAR },
					{ "ampersand", SDLK_AMPERSAND },
					{ "quote", SDLK_QUOTE },
					{ "leftparen", SDLK_LEFTPAREN },
					{ "rightparen", SDLK_RIGHTPAREN },
					{ "asterisk", SDLK_ASTERISK },
					{ "plus", SDLK_PLUS },
					{ "comma", SDLK_COMMA },
					{ "minus", SDLK_MINUS },
					{ "period", SDLK_PERIOD },
					{ "slash", SDLK_SLASH },
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
					{ "colon", SDLK_COLON },
					{ "semicolon", SDLK_SEMICOLON },
					{ "less", SDLK_LESS },
					{ "equals", SDLK_EQUALS },
					{ "greater", SDLK_GREATER },
					{ "question", SDLK_QUESTION },
					{ "at", SDLK_AT },
					{ "leftbracket", SDLK_LEFTBRACKET },
					{ "backslash", SDLK_BACKSLASH },
					{ "rightbracket", SDLK_RIGHTBRACKET },
					{ "caret", SDLK_CARET },
					{ "underscore", SDLK_UNDERSCORE },
					{ "backquote", SDLK_BACKQUOTE },
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
					{ "capslock", SDLK_CAPSLOCK },
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
					{ "printscreen", SDLK_PRINTSCREEN },
					{ "scrolllock", SDLK_SCROLLLOCK },
					{ "pause", SDLK_PAUSE },
					{ "insert", SDLK_INSERT },
					{ "home", SDLK_HOME },
					{ "pageup", SDLK_PAGEUP },
					{ "delete", SDLK_DELETE },
					{ "end", SDLK_END },
					{ "pagedown", SDLK_PAGEDOWN },
					{ "right", SDLK_RIGHT },
					{ "left", SDLK_LEFT },
					{ "down", SDLK_DOWN },
					{ "up", SDLK_UP },
					{ "numlockclear", SDLK_NUMLOCKCLEAR },
					{ "kp_divide", SDLK_KP_DIVIDE },
					{ "kp_multiply", SDLK_KP_MULTIPLY },
					{ "kp_minus", SDLK_KP_MINUS },
					{ "kp_plus", SDLK_KP_PLUS },
					{ "kp_enter", SDLK_KP_ENTER },
					{ "kp_1", SDLK_KP_1 },
					{ "kp_2", SDLK_KP_2 },
					{ "kp_3", SDLK_KP_3 },
					{ "kp_4", SDLK_KP_4 },
					{ "kp_5", SDLK_KP_5 },
					{ "kp_6", SDLK_KP_6 },
					{ "kp_7", SDLK_KP_7 },
					{ "kp_8", SDLK_KP_8 },
					{ "kp_9", SDLK_KP_9 },
					{ "kp_0", SDLK_KP_0 },
					{ "kp_period", SDLK_KP_PERIOD },
					{ "application", SDLK_APPLICATION },
					{ "power", SDLK_POWER },
					{ "kp_equals", SDLK_KP_EQUALS },
					{ "f13", SDLK_F13 },
					{ "f14", SDLK_F14 },
					{ "f15", SDLK_F15 },
					{ "f16", SDLK_F16 },
					{ "f17", SDLK_F17 },
					{ "f18", SDLK_F18 },
					{ "f19", SDLK_F19 },
					{ "f20", SDLK_F20 },
					{ "f21", SDLK_F21 },
					{ "f22", SDLK_F22 },
					{ "f23", SDLK_F23 },
					{ "f24", SDLK_F24 },
					{ "execute", SDLK_EXECUTE },
					{ "help", SDLK_HELP },
					{ "menu", SDLK_MENU },
					{ "select", SDLK_SELECT },
					{ "stop", SDLK_STOP },
					{ "again", SDLK_AGAIN },
					{ "undo", SDLK_UNDO },
					{ "cut", SDLK_CUT },
					{ "copy", SDLK_COPY },
					{ "paste", SDLK_PASTE },
					{ "find", SDLK_FIND },
					{ "mute", SDLK_MUTE },
					{ "volumeup", SDLK_VOLUMEUP },
					{ "volumedown", SDLK_VOLUMEDOWN },
					{ "kp_comma", SDLK_KP_COMMA },
					{ "kp_equalsas400", SDLK_KP_EQUALSAS400 },
					{ "alterase", SDLK_ALTERASE },
					{ "sysreq", SDLK_SYSREQ },
					{ "cancel", SDLK_CANCEL },
					{ "clear", SDLK_CLEAR },
					{ "prior", SDLK_PRIOR },
					{ "return2", SDLK_RETURN2 },
					{ "separator", SDLK_SEPARATOR },
					{ "out", SDLK_OUT },
					{ "oper", SDLK_OPER },
					{ "clearagain", SDLK_CLEARAGAIN },
					{ "crsel", SDLK_CRSEL },
					{ "exsel", SDLK_EXSEL },
					{ "kp_00", SDLK_KP_00 },
					{ "kp_000", SDLK_KP_000 },
					{ "thousandsseparator", SDLK_THOUSANDSSEPARATOR },
					{ "decimalseparator", SDLK_DECIMALSEPARATOR },
					{ "currencyunit", SDLK_CURRENCYUNIT },
					{ "currencysubunit", SDLK_CURRENCYSUBUNIT },
					{ "kp_leftparen", SDLK_KP_LEFTPAREN },
					{ "kp_rightparen", SDLK_KP_RIGHTPAREN },
					{ "kp_leftbrace", SDLK_KP_LEFTBRACE },
					{ "kp_rightbrace", SDLK_KP_RIGHTBRACE },
					{ "kp_tab", SDLK_KP_TAB },
					{ "kp_backspace", SDLK_KP_BACKSPACE },
					{ "kp_a", SDLK_KP_A },
					{ "kp_b", SDLK_KP_B },
					{ "kp_c", SDLK_KP_C },
					{ "kp_d", SDLK_KP_D },
					{ "kp_e", SDLK_KP_E },
					{ "kp_f", SDLK_KP_F },
					{ "kp_xor", SDLK_KP_XOR },
					{ "kp_power", SDLK_KP_POWER },
					{ "kp_percent", SDLK_KP_PERCENT },
					{ "kp_less", SDLK_KP_LESS },
					{ "kp_greater", SDLK_KP_GREATER },
					{ "kp_ampersand", SDLK_KP_AMPERSAND },
					{ "kp_dblampersand", SDLK_KP_DBLAMPERSAND },
					{ "kp_verticalbar", SDLK_KP_VERTICALBAR },
					{ "kp_dblverticalbar", SDLK_KP_DBLVERTICALBAR },
					{ "kp_colon", SDLK_KP_COLON },
					{ "kp_hash", SDLK_KP_HASH },
					{ "kp_space", SDLK_KP_SPACE },
					{ "kp_at", SDLK_KP_AT },
					{ "kp_exclam", SDLK_KP_EXCLAM },
					{ "kp_memstore", SDLK_KP_MEMSTORE },
					{ "kp_memrecall", SDLK_KP_MEMRECALL },
					{ "kp_memclear", SDLK_KP_MEMCLEAR },
					{ "kp_memadd", SDLK_KP_MEMADD },
					{ "kp_memsubtract", SDLK_KP_MEMSUBTRACT },
					{ "kp_memmultiply", SDLK_KP_MEMMULTIPLY },
					{ "kp_memdivide", SDLK_KP_MEMDIVIDE },
					{ "kp_plusminus", SDLK_KP_PLUSMINUS },
					{ "kp_clear", SDLK_KP_CLEAR },
					{ "kp_clearentry", SDLK_KP_CLEARENTRY },
					{ "kp_binary", SDLK_KP_BINARY },
					{ "kp_octal", SDLK_KP_OCTAL },
					{ "kp_decimal", SDLK_KP_DECIMAL },
					{ "kp_hexadecimal", SDLK_KP_HEXADECIMAL },
					{ "lctrl", SDLK_LCTRL },
					{ "lshift", SDLK_LSHIFT },
					{ "lalt", SDLK_LALT },
					{ "lgui", SDLK_LGUI },
					{ "rctrl", SDLK_RCTRL },
					{ "rshift", SDLK_RSHIFT },
					{ "ralt", SDLK_RALT },
					{ "rgui", SDLK_RGUI },
					{ "mode", SDLK_MODE },
					{ "audionext", SDLK_AUDIONEXT },
					{ "audioprev", SDLK_AUDIOPREV },
					{ "audiostop", SDLK_AUDIOSTOP },
					{ "audioplay", SDLK_AUDIOPLAY },
					{ "audiomute", SDLK_AUDIOMUTE },
					{ "mediaselect", SDLK_MEDIASELECT },
					{ "www", SDLK_WWW },
					{ "mail", SDLK_MAIL },
					{ "calculator", SDLK_CALCULATOR },
					{ "computer", SDLK_COMPUTER },
					{ "ac_search", SDLK_AC_SEARCH },
					{ "ac_home", SDLK_AC_HOME },
					{ "ac_back", SDLK_AC_BACK },
					{ "ac_forward", SDLK_AC_FORWARD },
					{ "ac_stop", SDLK_AC_STOP },
					{ "ac_refresh", SDLK_AC_REFRESH },
					{ "ac_bookmarks", SDLK_AC_BOOKMARKS },
					{ "brightnessdown", SDLK_BRIGHTNESSDOWN },
					{ "brightnessup", SDLK_BRIGHTNESSUP },
					{ "displayswitch", SDLK_DISPLAYSWITCH },
					{ "kbdillumtoggle", SDLK_KBDILLUMTOGGLE },
					{ "kbdillumdown", SDLK_KBDILLUMDOWN },
					{ "kbdillumup", SDLK_KBDILLUMUP },
					{ "eject", SDLK_EJECT },
					{ "sleep", SDLK_SLEEP },
					{ "app1", SDLK_APP1 },
					{ "app2", SDLK_APP2 },
					{ "audiorewind", SDLK_AUDIOREWIND },
					{ "audiofastforward", SDLK_AUDIOFASTFORWARD },

					// not available in SDL_keycode.h

					{ "half", 189 },
					{ nullptr, 0 }
				};

				for (int i = 0; pair[i].m_KeyName != nullptr; ++i)
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