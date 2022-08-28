#include "runtime/editor/utilities/editor_utils.h"

#include "runtime/editor/auxilarydata/auxilary_data_collection.h"
#include "runtime/editor/auxilarydata/auxilary_data_songs.h"
#include "runtime/editor/auxilarydata/auxilary_data_table_text.h"
#include "runtime/editor/components/component_table_row_elements.h"
#include "runtime/editor/driver/driver_info.h"
#include "runtime/editor/components_manager.h"
#include "runtime/emulation/cpumemory.h"
#include "foundation/input/keyboard_utils.h"
#include "foundation/base/assert.h"

namespace Editor
{
	namespace EditorUtils
	{
		static std::string g_noteKeyListOctave1 = "zsxdcvgbhnjm,l.";
		static std::string g_noteKeyListOctave2 = "q2w3er5t6y7ui9o0p";

		void SetNoteValueKeys(const std::string& inNoteKeyListOctave1, const std::string& inNoteKeyListOctave2)
		{
			g_noteKeyListOctave1 = inNoteKeyListOctave1;
			g_noteKeyListOctave2 = inNoteKeyListOctave2;
		}

		bool Has2ndInputOctave()
		{
			return !g_noteKeyListOctave2.empty();
		}

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
			int note_value = [&](char inKey)
			{
				const size_t note_value_octave1 = g_noteKeyListOctave1.find_first_of(inKey);
				if (note_value_octave1 != -1)
					return static_cast<int>(note_value_octave1);

				const size_t note_value_octave2 = g_noteKeyListOctave2.find_first_of(inKey);
				if (note_value_octave2 != -1)
					return static_cast<int>(note_value_octave2) + 12;

				return -1;
			}(inKey);

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


		unsigned char GetTableIDFromNameInTableDefinition(const DriverInfo& inDriverInfo, const std::string& inTableName)
		{
			const auto& table_definitions = inDriverInfo.GetTableDefinitions();

			const auto is_same = [](const std::string& inName1, const std::string& inName2)
			{
				if (inName1.size() != inName2.size())
					return false;

				const char* c_string1 = inName1.c_str();
				const char* c_string2 = inName2.c_str();

				for (size_t i = 0; i < inName1.size(); ++i)
				{
					if (std::tolower(c_string1[i]) != std::tolower(c_string2[i]))
						return false;
				}

				return true;
			};

			for (const auto& table_definition : table_definitions)
			{
				if (is_same(table_definition.m_Name, inTableName))
					return table_definition.m_ID;
			}

			return 0xff;
		}


		void UpdateSongNameOfSingleSongPackages(DriverInfo& inDriverInfo)
		{
			const unsigned int song_count = static_cast<unsigned int>(inDriverInfo.GetAuxilaryDataCollection().GetSongs().GetSongCount());
			FOUNDATION_ASSERT(song_count > 0);

			if (song_count == 1)
			{
				const std::string& song_name = inDriverInfo.GetAuxilaryDataCollection().GetSongs().GetSongName(0);
				if (song_name.empty())
					inDriverInfo.GetAuxilaryDataCollection().GetSongs().SetSongName(0, "Main");
			}
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

		void AddSong(const std::string& inName, DriverInfo& inDriverInfo, Emulation::CPUMemory& inCPUMemory, ComponentsManager* inComponentsManager, unsigned char inSongOverviewTableID)
		{
			const unsigned int song_count = static_cast<unsigned int>(inDriverInfo.GetAuxilaryDataCollection().GetSongs().GetSongCount());
			FOUNDATION_ASSERT(song_count > 0);

			const unsigned int new_song_index = song_count;

			if (song_count < MAX_SONG_COUNT)
			{
				inCPUMemory.Lock();

				const auto& music_data = inDriverInfo.GetMusicData();

				unsigned short song_order_list_byte_size = music_data.m_OrderListSize * music_data.m_TrackCount;
				unsigned short order_list_insert_address = music_data.m_OrderListTrack1Address + song_order_list_byte_size * (new_song_index);
				unsigned short length = music_data.m_SequenceSize * music_data.m_SequenceCount + song_order_list_byte_size * (song_count - (new_song_index - 1));

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

				if (inComponentsManager != nullptr)
				{
					const unsigned char init_table_id = GetTableIDFromNameInTableDefinition(inDriverInfo, "init");
					if (init_table_id != 0xff)
					{
						ComponentTableRowElements* component = reinterpret_cast<ComponentTableRowElements*>(inComponentsManager->GetComponent(init_table_id));
						FOUNDATION_ASSERT(component != nullptr);

						DataSourceTable* init_table_data_source = component->GetDataSource();
						init_table_data_source->PullDataFromSource();

						const int table_column_count = init_table_data_source->GetColumnCount();
						const int from_row = (new_song_index - 1) * table_column_count;

						for (int i = 0; i < table_column_count; ++i)
							(*init_table_data_source)[from_row + i + table_column_count] = (*init_table_data_source)[from_row + i];

						inCPUMemory.Lock();
						init_table_data_source->PushDataToSource();
						inCPUMemory.Unlock();
					}
				}

				inDriverInfo.RefreshMusicData(inCPUMemory);
				inDriverInfo.GetAuxilaryDataCollection().GetSongs().AddSong(new_song_index);
				inDriverInfo.GetAuxilaryDataCollection().GetSongs().SetSongName(new_song_index, inName);
				inDriverInfo.GetAuxilaryDataCollection().GetTableText().InsertLayer(static_cast<int>(inSongOverviewTableID), new_song_index);

				SelectSong(new_song_index, inDriverInfo, inCPUMemory);
			}
		}

		void RemoveSong(unsigned int inIndex, DriverInfo& inDriverInfo, Emulation::CPUMemory& inCPUMemory, ComponentsManager* inComponentsManager, unsigned char inSongOverviewTableID)
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

			if (inComponentsManager != nullptr)
			{
				const unsigned char init_table_id = GetTableIDFromNameInTableDefinition(inDriverInfo, "init");
				if (init_table_id != 0xff)
				{
					ComponentTableRowElements* component = reinterpret_cast<ComponentTableRowElements*>(inComponentsManager->GetComponent(init_table_id));
					FOUNDATION_ASSERT(component != nullptr);

					DataSourceTable* init_table_data_source = component->GetDataSource();
					init_table_data_source->PullDataFromSource();

					const int table_column_count = init_table_data_source->GetColumnCount();

					const int from_row = inIndex + 1;
					const int to_row = inIndex;
					const int rows_to_move_count = init_table_data_source->GetRowCount() - from_row;
					const int column_count = init_table_data_source->GetColumnCount();

					int index = from_row * column_count;

					for (int i = 0; i < rows_to_move_count; ++i)
					{
						for (int j = 0; j < column_count; ++j)
							(*init_table_data_source)[index - column_count + j] = (*init_table_data_source)[index + j];

						index += column_count;
					}

					inCPUMemory.Lock();
					init_table_data_source->PushDataToSource();
					inCPUMemory.Unlock();
				}
			}

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

			inDriverInfo.GetAuxilaryDataCollection().GetSongs().SetSongName(inIndex, inNewName);
		}


		void SwapSongs(unsigned int inIndex1, unsigned int inIndex2, DriverInfo& inDriverInfo, Emulation::CPUMemory& inCPUMemory, unsigned char inSongOverviewTableID, ComponentsManager& inComponentsManager)
		{
			const unsigned int song_count = static_cast<unsigned int>(inDriverInfo.GetAuxilaryDataCollection().GetSongs().GetSongCount());

			FOUNDATION_ASSERT(inIndex1 < song_count);
			FOUNDATION_ASSERT(inIndex2 < song_count);

			if (inIndex1 == inIndex2)
				return;

			inCPUMemory.Lock();

			const auto& music_data = inDriverInfo.GetMusicData();

			unsigned short song_order_list_byte_size = music_data.m_OrderListSize * music_data.m_TrackCount;

			unsigned short order_list_index1_address = music_data.m_OrderListTrack1Address + song_order_list_byte_size * inIndex1;
			unsigned short order_list_index2_address = music_data.m_OrderListTrack1Address + song_order_list_byte_size * inIndex2;

			unsigned char* order_list_index1_buffer = new unsigned char[song_order_list_byte_size];
			unsigned char* order_list_index2_buffer = new unsigned char[song_order_list_byte_size];

			inCPUMemory.GetData(order_list_index1_address, order_list_index1_buffer, song_order_list_byte_size);
			inCPUMemory.GetData(order_list_index2_address, order_list_index2_buffer, song_order_list_byte_size);
			inCPUMemory.SetData(order_list_index1_address, order_list_index2_buffer, song_order_list_byte_size);
			inCPUMemory.SetData(order_list_index2_address, order_list_index1_buffer, song_order_list_byte_size);

			delete[] order_list_index1_buffer;
			delete[] order_list_index2_buffer;

			inCPUMemory.Unlock();

			const unsigned char init_table_id = GetTableIDFromNameInTableDefinition(inDriverInfo, "init");
			if (init_table_id != 0xff)
			{
				ComponentTableRowElements* component = reinterpret_cast<ComponentTableRowElements*>(inComponentsManager.GetComponent(init_table_id));
				FOUNDATION_ASSERT(component != nullptr);

				DataSourceTable* init_table_data_source = component->GetDataSource();
				init_table_data_source->PullDataFromSource();

				const int table_column_count = init_table_data_source->GetColumnCount();
				const int row_1 = (inIndex1) * table_column_count;
				const int row_2 = (inIndex2) * table_column_count;


				for (int i = 0; i < table_column_count; ++i)
				{
					unsigned char row_1_value = (*init_table_data_source)[row_1 + i];
					(*init_table_data_source)[row_1 + i] = (*init_table_data_source)[row_2 + i];
					(*init_table_data_source)[row_2 + i] = row_1_value;
				}


				inCPUMemory.Lock();
				init_table_data_source->PushDataToSource();
				inCPUMemory.Unlock();

			}

			inDriverInfo.GetAuxilaryDataCollection().GetSongs().SwapSongs(static_cast<unsigned char>(inIndex1), static_cast<unsigned char>(inIndex2));
			inDriverInfo.GetAuxilaryDataCollection().GetTableText().SwapLayers(inSongOverviewTableID, static_cast<unsigned char>(inIndex1), static_cast<unsigned char>(inIndex2));
		}


		bool MoveSong(unsigned int inIndexFrom, unsigned int inIndexTo, DriverInfo& inDriverInfo, Emulation::CPUMemory& inCPUMemory, unsigned char inSongOverviewTableID, ComponentsManager& inComponentsManager)
		{
			if (inIndexFrom == inIndexTo)
				return false;

			const int move_delta = static_cast<int>(inIndexTo) - static_cast<int>(inIndexFrom);
			
			SwapSongs(inIndexFrom, inIndexTo, inDriverInfo, inCPUMemory, inSongOverviewTableID, inComponentsManager);
			
			if (std::abs(move_delta) == 1)
				return true;

			if (move_delta < 0)
			{
				int index = inIndexFrom;
				const int steps = (-move_delta) - 1;

				for (int i = 0; i < steps; ++i)
				{
					SwapSongs(index, index - 1, inDriverInfo, inCPUMemory, inSongOverviewTableID, inComponentsManager);
					--index;
				}
			}
			else
			{
				int index = inIndexFrom;
				const int steps = move_delta - 1;

				for (int i = 0; i < steps; ++i)
				{
					SwapSongs(index, index + 1, inDriverInfo, inCPUMemory, inSongOverviewTableID, inComponentsManager);
					++index;
				}
			}

			SelectSong(inIndexTo, inDriverInfo, inCPUMemory);

			return true;
		}
	}
}