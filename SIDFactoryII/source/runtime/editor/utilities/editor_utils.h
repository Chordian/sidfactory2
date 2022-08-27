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
	class ComponentsManager;

	namespace EditorUtils
	{
		static constexpr int MAX_SONG_COUNT = 0x10;

		int GetNoteValue(SDL_Keycode inKeyCode, int inOctave);
		int GetNoteValue(char inKeyCharacter, int inOctave);
		unsigned char ConvertSingleCharHexValueToValue(char inKeyCharacter);
		char ConvertValueToSingleCharHexValue(unsigned char inValue, bool inUppercase);
		std::string ConvertToHexValue(unsigned char inValue, bool inUppercase);
		std::string ConvertToHexValue(unsigned short inValue, bool inUppercase);

		unsigned char GetTableIDFromNameInTableDefinition(const DriverInfo& inDriverinfo, const std::string& inTableName);

		void SelectSong(unsigned int inIndex, DriverInfo& inDriverInfo, Emulation::CPUMemory& inCPUMemory);
		void AddSong(const std::string& inName, DriverInfo& inDriverInfo, Emulation::CPUMemory& inCPUMemory, ComponentsManager* inComponentsManager, unsigned char inSongOverviewTableID);
		void RemoveSong(unsigned int inIndex, DriverInfo& inDriverInfo, Emulation::CPUMemory& inCPUMemory, ComponentsManager* inComponentsManager, unsigned char inSongOverviewTableID);
		void RenameSong(unsigned int inIndex, const std::string& inNewName, DriverInfo& inDriverInfo);
		bool MoveSong(unsigned int inIndexFrom, unsigned int inIndexTo, DriverInfo& inDriverInfo, Emulation::CPUMemory& inCPUMemory, unsigned char inSongOverviewTableID, ComponentsManager& inComponentsManager);
	}
}