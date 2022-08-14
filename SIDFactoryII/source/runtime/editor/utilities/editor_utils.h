#pragma once

#include "SDL_keyboard.h"
#include <string>

namespace Emulation
{
	class CPUMemory;
}

namespace Editor
{
	class DriverInfo;

	namespace EditorUtils
	{
		int GetNoteValue(SDL_Keycode inKeyCode, int inOctave);
		int GetNoteValue(char inKeyCharacter, int inOctave);
		unsigned char ConvertSingleCharHexValueToValue(char inKeyCharacter);
		char ConvertValueToSingleCharHexValue(unsigned char inValue, bool inUppercase);
		std::string ConvertToHexValue(unsigned char inValue, bool inUppercase);
		std::string ConvertToHexValue(unsigned short inValue, bool inUppercase);

		void SelectSong(unsigned int inIndex, DriverInfo& inDriverInfo, Emulation::CPUMemory& inCPUMemory);
		void AddSong(unsigned int inIndex, DriverInfo& inDriverInfo, Emulation::CPUMemory& inCPUMemory);
		void RemoveSong(unsigned int inIndex, DriverInfo& inDriverInfo, Emulation::CPUMemory& inCPUMemory);
	}
}