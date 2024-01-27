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

		// Function for note inputs
		void SetNoteInputValueKeys(const std::string& inNoteKeyListOctave1, const std::string& inNoteKeyListOctave2);
		bool Has2ndNoteInputOctave();
		
		int GetNoteValue(SDL_Keycode inKeyCode, int inOctave);
		int GetNoteValue(char inKeyCharacter, int inOctave);

		// Hex conversion functions
		unsigned char ConvertSingleCharHexValueToValue(char inKeyCharacter);
		char ConvertValueToSingleCharHexValue(unsigned char inValue, bool inUppercase);
		std::string ConvertToHexValue(unsigned char inValue, bool inUppercase);
		std::string ConvertToHexValue(unsigned short inValue, bool inUppercase);

		// Table ID query functions
		unsigned char GetTableIDFromNameInTableDefinition(const DriverInfo& inDriverinfo, const std::string& inTableName);

		// Utility methods for upgrading to multi song packages
		void UpdateSongNameOfSingleSongPackages(DriverInfo& inDriverInfo);
		void AddMissingPlayerMarkerLayers(DriverInfo& inDriverInfo);

		// Multi song utility functions
		void SelectSong(unsigned int inIndex, DriverInfo& inDriverInfo, Emulation::CPUMemory& inCPUMemory);
		void AddSong(const std::string& inName, DriverInfo& inDriverInfo, Emulation::CPUMemory& inCPUMemory, ComponentsManager* inComponentsManager, unsigned char inSongOverviewTableID);
		void RemoveSong(unsigned int inIndex, DriverInfo& inDriverInfo, Emulation::CPUMemory& inCPUMemory, ComponentsManager* inComponentsManager, unsigned char inSongOverviewTableID);
		void RenameSong(unsigned int inIndex, const std::string& inNewName, DriverInfo& inDriverInfo);
		bool MoveSong(unsigned int inIndexFrom, unsigned int inIndexTo, DriverInfo& inDriverInfo, Emulation::CPUMemory& inCPUMemory, unsigned char inSongOverviewTableID, ComponentsManager& inComponentsManager);
	}
}