#include "runtime/editor/utilities/editor_utils.h"

#include "runtime/editor/auxilarydata/auxilary_data_collection.h"
#include "runtime/editor/auxilarydata/auxilary_data_songs.h"
#include "runtime/editor/auxilarydata/auxilary_data_table_text.h"
#include "runtime/editor/driver/driver_info.h"
#include "runtime/emulation/cpumemory.h"
#include "foundation/input/keyboard_utils.h"
#include "foundation/base/assert.h"

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

			FOUNDATION_ASSERT(true);

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

			FOUNDATION_ASSERT(true);

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


		void SelectSong(unsigned int inIndex, DriverInfo& inDriverInfo, Emulation::CPUMemory& inCPUMemory)
		{
			const unsigned int song_count = static_cast<unsigned int>(inDriverInfo.GetAuxilaryDataCollection().GetSongs().GetSongCount());
			FOUNDATION_ASSERT(song_count > 0);
			FOUNDATION_ASSERT(inIndex < song_count);

			inDriverInfo.GetAuxilaryDataCollection().GetSongs().SetSelectedSong(inIndex);

			inCPUMemory.Lock();

			const auto& music_data = inDriverInfo.GetMusicData();

			unsigned short song_order_list_byte_size = music_data.m_OrderListSize * music_data.m_TrackCount;
			const auto first_song_orderlist_address = music_data.m_OrderListTrack1Address + song_order_list_byte_size * inIndex;

			for (unsigned char i = 0; i < music_data.m_TrackCount; ++i)
			{
				unsigned short address = first_song_orderlist_address + music_data.m_OrderListSize * i;
				inCPUMemory.SetByte(music_data.m_TrackOrderListPointersLowAddress + i, static_cast<unsigned char>(address & 0xff));
				inCPUMemory.SetByte(music_data.m_TrackOrderListPointersHighAddress + i, static_cast<unsigned char>(address >> 8));
			}

			inCPUMemory.Unlock();
		}

		void AddSong(unsigned int inIndex, const std::string& inName, DriverInfo& inDriverInfo, Emulation::CPUMemory& inCPUMemory, unsigned char inSongOverviewTableID)
		{
			const unsigned int song_count = static_cast<unsigned int>(inDriverInfo.GetAuxilaryDataCollection().GetSongs().GetSongCount());
			FOUNDATION_ASSERT(song_count > 0);
			FOUNDATION_ASSERT(inIndex < song_count);

			if (song_count < 0x10)
			{
				FOUNDATION_ASSERT(inIndex < inDriverInfo.GetAuxilaryDataCollection().GetSongs().GetSongCount());

				inCPUMemory.Lock();

				const auto& music_data = inDriverInfo.GetMusicData();

				unsigned short song_order_list_byte_size = music_data.m_OrderListSize * music_data.m_TrackCount;
				unsigned short order_list_insert_address = music_data.m_OrderListTrack1Address + song_order_list_byte_size * (inIndex + 1);
				unsigned short length = music_data.m_SequenceSize * music_data.m_SequenceCount + song_order_list_byte_size * (song_count - inIndex);

				// Move orderlists and sequences down in memory to make room for orderlist of the new song
				inCPUMemory.Copy(order_list_insert_address, length, order_list_insert_address + song_order_list_byte_size);
				inCPUMemory.Set(0, order_list_insert_address, song_order_list_byte_size);

				// Set default orderlist value for the inserted song
				static const unsigned char default_order_list[] = { 0xa0, 0x00, 0xff, 0x00 };
				
				for (unsigned char i = 0; i < music_data.m_TrackCount; ++i)
					inCPUMemory.SetData(order_list_insert_address + i * music_data.m_OrderListSize, default_order_list, sizeof(default_order_list));

				// Update all sequence points
				for (unsigned int i = 0; i < music_data.m_SequenceCount; ++i)
				{
					unsigned short sequence_address = static_cast<unsigned short>(inCPUMemory.GetByte(music_data.m_SequencePointersLowAddress + i)) |
						(static_cast<unsigned short>(inCPUMemory.GetByte(music_data.m_SequencePointersHighAddress + i)) << 8);

					sequence_address += song_order_list_byte_size;

					unsigned char sequence_address_low = static_cast<unsigned char>(sequence_address & 0xff);
					unsigned char sequence_address_high = static_cast<unsigned char>(sequence_address >> 8);

					inCPUMemory.SetByte(music_data.m_SequencePointersLowAddress + i, sequence_address_low);
					inCPUMemory.SetByte(music_data.m_SequencePointersHighAddress + i, sequence_address_high);
				}

				const auto& music_data_meta_data_addresses_in_emulation_memory = inDriverInfo.GetMusicDataMetaDataEmulationAddresses();

				unsigned short address_of_first_sequence = inCPUMemory.GetWord(music_data_meta_data_addresses_in_emulation_memory.m_EmulationAddressOfSequence00Address);
				inCPUMemory.SetWord(music_data_meta_data_addresses_in_emulation_memory.m_EmulationAddressOfSequence00Address, address_of_first_sequence + song_order_list_byte_size);

				inCPUMemory.Unlock();

				inDriverInfo.RefreshMusicData(inCPUMemory);
				inDriverInfo.GetAuxilaryDataCollection().GetSongs().AddSong(inIndex + 1);
				inDriverInfo.GetAuxilaryDataCollection().GetSongs().SetSongName(inIndex + 1, inName);
				inDriverInfo.GetAuxilaryDataCollection().GetTableText().InsertLayer(static_cast<int>(inSongOverviewTableID), inIndex + 1);

				SelectSong(inIndex + 1, inDriverInfo, inCPUMemory);
			}
		}

		void RemoveSong(unsigned int inIndex, DriverInfo& inDriverInfo, Emulation::CPUMemory& inCPUMemory, unsigned char inSongOverviewTableID)
		{
			const unsigned int song_count = static_cast<unsigned int>(inDriverInfo.GetAuxilaryDataCollection().GetSongs().GetSongCount());
			if (song_count < 2)
				return;

			const unsigned int selected_song = static_cast<unsigned int>(inDriverInfo.GetAuxilaryDataCollection().GetSongs().GetSelectedSong());
			FOUNDATION_ASSERT(inIndex < song_count);

			inCPUMemory.Lock();

			const auto& music_data = inDriverInfo.GetMusicData();

			unsigned short song_order_list_byte_size = music_data.m_OrderListSize * music_data.m_TrackCount;
			unsigned short order_list_copy_from_address = music_data.m_OrderListTrack1Address + song_order_list_byte_size * (inIndex + 1);
			unsigned short order_list_copy_to_address = order_list_copy_from_address - song_order_list_byte_size;

			unsigned short length = music_data.m_SequenceSize * music_data.m_SequenceCount + song_order_list_byte_size * (song_count - (inIndex + 1));

			// Move orderlists and sequences down in memory to make room for orderlist of the new song
			inCPUMemory.Copy(order_list_copy_from_address, length, order_list_copy_to_address);

			// Update all sequence pointers
			for (unsigned int i = 0; i < music_data.m_SequenceCount; ++i)
			{
				unsigned short sequence_address = static_cast<unsigned short>(inCPUMemory.GetByte(music_data.m_SequencePointersLowAddress + i)) |
					(static_cast<unsigned short>(inCPUMemory.GetByte(music_data.m_SequencePointersHighAddress + i)) << 8);

				sequence_address -= song_order_list_byte_size;

				unsigned char sequence_address_low = static_cast<unsigned char>(sequence_address & 0xff);
				unsigned char sequence_address_high = static_cast<unsigned char>(sequence_address >> 8);

				inCPUMemory.SetByte(music_data.m_SequencePointersLowAddress + i, sequence_address_low);
				inCPUMemory.SetByte(music_data.m_SequencePointersHighAddress + i, sequence_address_high);
			}

			const auto& music_data_meta_data_addresses_in_emulation_memory = inDriverInfo.GetMusicDataMetaDataEmulationAddresses();

			unsigned short address_of_first_sequence = inCPUMemory.GetWord(music_data_meta_data_addresses_in_emulation_memory.m_EmulationAddressOfSequence00Address);
			inCPUMemory.SetWord(music_data_meta_data_addresses_in_emulation_memory.m_EmulationAddressOfSequence00Address, address_of_first_sequence - song_order_list_byte_size);

			inCPUMemory.Unlock();

			inDriverInfo.RefreshMusicData(inCPUMemory);
			inDriverInfo.GetAuxilaryDataCollection().GetSongs().RemoveSong(inIndex);
			inDriverInfo.GetAuxilaryDataCollection().GetTableText().RemoveLayer(static_cast<int>(inSongOverviewTableID), inIndex);

			if (selected_song == inIndex)
			{
				if (inIndex < song_count - 1)
					SelectSong(inIndex, inDriverInfo, inCPUMemory);
				else
					SelectSong(selected_song - 1, inDriverInfo, inCPUMemory);
			}
			else if (selected_song > inIndex)
				SelectSong(selected_song - 1, inDriverInfo, inCPUMemory);
			else
				SelectSong(selected_song, inDriverInfo, inCPUMemory);
		}


		void RenameSong(unsigned int inIndex, const std::string& inNewName, DriverInfo& inDriverInfo)
		{
			const unsigned int song_count = static_cast<unsigned int>(inDriverInfo.GetAuxilaryDataCollection().GetSongs().GetSongCount());
			FOUNDATION_ASSERT(inIndex < song_count);

			// TODO: Implement
			FOUNDATION_ASSERT(false);
		}


		void SwapSongs(unsigned int inIndex1, unsigned int inIndex2, DriverInfo& inDriverInfo, Emulation::CPUMemory& inCPUMemory, unsigned char inSongOverviewTableID)
		{
			const unsigned int song_count = static_cast<unsigned int>(inDriverInfo.GetAuxilaryDataCollection().GetSongs().GetSongCount());

			FOUNDATION_ASSERT(inIndex1 < song_count);
			FOUNDATION_ASSERT(inIndex2 < song_count);

			if (inIndex1 == inIndex2)
				return;

			// TODO: Implement swap table text layers for the song over view text buffers
			// TODO: Implement swap song orderlists

			FOUNDATION_ASSERT(false);

			inDriverInfo.GetAuxilaryDataCollection().GetSongs().SwapSongs(static_cast<unsigned char>(inIndex1), static_cast<unsigned char>(inIndex2));
		}
	}
}