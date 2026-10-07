#include "runtime/editor/driver/driver_utils.h"
#include "runtime/editor/datasources/datasource_table.h"
#include "runtime/editor/datasources/datasource_table_column_major.h"
#include "runtime/editor/datasources/datasource_table_row_major.h"
#include "runtime/editor/driver/driver_info.h"
#include "runtime/editor/auxilarydata/auxilary_data_collection.h"
#include "runtime/editor/auxilarydata/auxilary_data_songs.h"
#include "runtime/emulation/cpumemory.h"
#include "runtime/emulation/imemoryrandomreadaccess.h"
#include "runtime/emulation/cpumos6510.h"
#include "runtime/emulation/cpuframecapture.h"
#include "runtime/environmentdefines.h"
#include "utils/global.h"
#include "utils/c64file.h"
#include "foundation/base/assert.h"

#include <algorithm>
#include <unordered_map>

namespace Editor
{
	namespace DriverUtils
	{
		std::shared_ptr<DataSourceTable> CreateTableDataSource(const DriverInfo::TableDefinition& inTableDefinition, Emulation::CPUMemory* inCPUMemory)
		{
			return (inTableDefinition.m_DataLayout == DriverInfo::TableDefinition::DataLayout::ColumnMajor)
				? std::shared_ptr<DataSourceTable>(new DataSourceTableColumnMajor(inCPUMemory, inTableDefinition.m_Address, inTableDefinition.m_RowCount, inTableDefinition.m_ColumnCount))
				: std::shared_ptr<DataSourceTable>(new DataSourceTableRowMajor(inCPUMemory, inTableDefinition.m_Address, inTableDefinition.m_RowCount, inTableDefinition.m_ColumnCount));
		}


		unsigned char GetHighestSequenceIndexUsed(const Editor::DriverInfo& inDriverInfo, const Emulation::IMemoryRandomReadAccess& inMemoryReader)
		{
			unsigned char highest_sequence_index = 0;

			if (inDriverInfo.HasParsedHeaderBlock(DriverInfo::HeaderBlockID::ID_MusicData))
			{
				// Get song count
				const unsigned char song_count = inDriverInfo.GetAuxilaryDataCollection().GetSongs().GetSongCount();

				// Get Music data descriptor
				const DriverInfo::MusicData& music_data = inDriverInfo.GetMusicData();

				// Find highest used sequence index
				unsigned short order_list_1 = music_data.m_OrderListTrack1Address;

				for (unsigned char i = 0; i < music_data.m_TrackCount * song_count; ++i)
				{
					unsigned short order_list_address = order_list_1 + static_cast<unsigned short>(i) * music_data.m_OrderListSize;
					for (unsigned short j = 0; j < music_data.m_OrderListSize; ++j)
					{
						unsigned char value = inMemoryReader[order_list_address + j];
						if (value < 0x80)
						{
							if (value > highest_sequence_index)
								highest_sequence_index = value;
						}
						else
						{
							if (value == 0xff)
								break;
						}
					}
				}
			}

			return highest_sequence_index;
		}


		std::vector<int> GetSequenceUsageCount(const Editor::DriverInfo& inDriverInfo, const Emulation::IMemoryRandomReadAccess& inMemoryReader)
		{
			if (inDriverInfo.HasParsedHeaderBlock(DriverInfo::HeaderBlockID::ID_MusicData))
			{
				// Output collection
				const int SequenceCount = static_cast<int>(inDriverInfo.GetMusicData().m_SequenceCount);
				std::vector<int> usage_count(SequenceCount, 0);

				// Get Music data descriptor
				const DriverInfo::MusicData& music_data = inDriverInfo.GetMusicData();

				// Find highest used sequence index
				unsigned short order_list_1 = music_data.m_OrderListTrack1Address;

				// Get song count
				const unsigned char song_count = inDriverInfo.GetAuxilaryDataCollection().GetSongs().GetSongCount();

				for (unsigned char i = 0; i < music_data.m_TrackCount * song_count; ++i)
				{
					unsigned short order_list_address = order_list_1 + static_cast<unsigned short>(i) * music_data.m_OrderListSize;
					for (unsigned short j = 0; j < music_data.m_OrderListSize; ++j)
					{
						unsigned char value = inMemoryReader[order_list_address + j];

						if (value < SequenceCount)
							usage_count[value]++;
						else if (value >= 0xfe)
							break;
					}
				}

				return usage_count;
			}

			return std::vector<int>();
		}


		unsigned char GetFirstUnusedSequenceIndex(const Editor::DriverInfo& inDriverInfo, const Emulation::IMemoryRandomReadAccess& inMemoryReader)
		{
			std::vector<int> sequence_usage_count = GetSequenceUsageCount(inDriverInfo, inMemoryReader);

			for (size_t i = 0; i < sequence_usage_count.size(); ++i)
			{
				if (sequence_usage_count[i] == 0)
					return static_cast<unsigned char>(i);
			}

			return 0xff;
		}

		unsigned char GetFirstEmptySequenceIndex(const Editor::DriverInfo& inDriverInfo, const Emulation::IMemoryRandomReadAccess& inMemoryReader)
		{
			if (inDriverInfo.HasParsedHeaderBlock(DriverInfo::HeaderBlockID::ID_MusicData))
			{
				const DriverInfo::MusicData& music_data = inDriverInfo.GetMusicData();
				const std::vector<int> sequence_usage_count = GetSequenceUsageCount(inDriverInfo, inMemoryReader);

				for (unsigned short i = 0; i < static_cast<unsigned short>(sequence_usage_count.size()); ++i)
				{
					const unsigned short sequence_address = music_data.m_Sequence00Address + i * music_data.m_SequenceSize;

					const bool is_empty = inMemoryReader[sequence_address] == 0x80
						&& inMemoryReader[sequence_address + 1] == 0x00
						&& inMemoryReader[sequence_address + 2] == 0x7f;

					if (is_empty && sequence_usage_count[i] == 0)
						return static_cast<unsigned char>(i);
				}
			}
			return 0xff;
		}


		unsigned char GetHighestInstrumentIndexUsed(const Editor::DriverInfo& inDriverInfo, const Emulation::IMemoryRandomReadAccess& inMemoryReader)
		{
			unsigned char highest_instrument_index = 0;

			if (inDriverInfo.HasParsedHeaderBlock(DriverInfo::HeaderBlockID::ID_MusicData))
			{
				// Get Music data descriptor
				const DriverInfo::MusicData& music_data = inDriverInfo.GetMusicData();

				// Find highest used sequence index
				unsigned short sequence_0 = music_data.m_Sequence00Address;

				for (unsigned char i = 0; i < music_data.m_SequenceCount; ++i)
				{
					unsigned short order_list_address = sequence_0 + static_cast<unsigned short>(i) * music_data.m_SequenceSize;

					for (unsigned short j = 0; j < music_data.m_SequenceSize; ++j)
					{
						const unsigned char value = inMemoryReader[order_list_address + j];
						if (value >= 0x80)
						{
							if (value >= 0xa0 && value < 0xc0)
							{
								const unsigned char instrument_index = value & 0x1f;
								if (instrument_index > highest_instrument_index)
									highest_instrument_index = instrument_index;
							}
						}
						else
						{
							if (value == 0x7f)
								break;
						}
					}
				}
			}

			return highest_instrument_index;
		}


		unsigned char GetHighestCommandIndexUsed(const Editor::DriverInfo& inDriverInfo, const Emulation::IMemoryRandomReadAccess& inMemoryReader)
		{
			unsigned char highest_command_index = 0;

			if (inDriverInfo.HasParsedHeaderBlock(DriverInfo::HeaderBlockID::ID_MusicData))
			{
				// Get Music data descriptor
				const DriverInfo::MusicData& music_data = inDriverInfo.GetMusicData();

				// Find highest used sequence index
				unsigned short sequence_0 = music_data.m_Sequence00Address;

				for (unsigned char i = 0; i < music_data.m_SequenceCount; ++i)
				{
					unsigned short order_list_address = sequence_0 + static_cast<unsigned short>(i) * music_data.m_SequenceSize;

					for (unsigned short j = 0; j < music_data.m_SequenceSize; ++j)
					{
						const unsigned char value = inMemoryReader[order_list_address + j];
						if (value >= 0x80)
						{
							if (value >= 0xc0)
							{
								const unsigned char command_index = value & 0x3f;
								if (command_index > highest_command_index)
									highest_command_index = command_index;
							}
						}
						else
						{
							if (value == 0x7f)
								break;
						}
					}
				}
			}

			return highest_command_index;
		}


		unsigned char GetHighestTableRowUsedIndex(const Editor::DriverInfo::TableDefinition& inTableDefinition, const Emulation::IMemoryRandomReadAccess& inMemoryReader)
		{
			unsigned short highest_used_index = 0;

			const unsigned short table_address = inTableDefinition.m_Address;

			if (inTableDefinition.m_DataLayout == Editor::DriverInfo::TableDefinition::DataLayout::ColumnMajor)
			{
				// Column major scan
				const unsigned short table_row_count = inTableDefinition.m_RowCount;
				for (unsigned short i = 0; i < table_row_count; ++i)
				{
					for (unsigned short j = 0; j < inTableDefinition.m_ColumnCount; ++j)
					{
						const unsigned short address = table_address + i + table_row_count * j;

						if (inMemoryReader[address] != 0)
						{
							highest_used_index = i;
							break;
						}
					}
				}
			}
			else if(inTableDefinition.m_DataLayout == Editor::DriverInfo::TableDefinition::DataLayout::RowMajor)
			{
				// Column major scan
				const unsigned short table_column_count = inTableDefinition.m_ColumnCount;
				for (unsigned short i = 0; i < inTableDefinition.m_RowCount; ++i)
				{
					for (unsigned short j = 0; j < table_column_count; ++j)
					{
						const unsigned short address = table_address + j + table_column_count * i;

						if (inMemoryReader[address] != 0)
						{
							highest_used_index = i;
							break;
						}
					}
				}
			}

			FOUNDATION_ASSERT(highest_used_index < 0x100);
			return static_cast<unsigned char>(highest_used_index);
		}


		std::vector<unsigned short> GetOrderListsLength(const Editor::DriverInfo& inDriverInfo, const Emulation::IMemoryRandomReadAccess& inMemoryReader)
		{
			std::vector<unsigned short> order_list_length_list;

			if (inDriverInfo.HasParsedHeaderBlock(DriverInfo::HeaderBlockID::ID_MusicData))
			{
				// Get song count
				const unsigned char song_count = inDriverInfo.GetAuxilaryDataCollection().GetSongs().GetSongCount();

				// Get Music data descriptor
				const DriverInfo::MusicData& music_data = inDriverInfo.GetMusicData();

				// Find highest used sequence index
				unsigned short order_list_1 = music_data.m_OrderListTrack1Address;

				for (unsigned char i = 0; i < music_data.m_TrackCount * song_count; ++i)
				{
					unsigned short order_list_address = order_list_1 + static_cast<unsigned short>(i) * music_data.m_OrderListSize;
					for (int j = 0; j < music_data.m_OrderListSize; ++j)
					{
						unsigned char value = inMemoryReader[order_list_address + j];
						if (value == 0xff)
						{
							// +2, because the value after $ff is used as the loop index of the order list!
							order_list_length_list.push_back(static_cast<unsigned short>(j) + 2);
							break;
						}
						else if (value == 0xfe)
						{
							order_list_length_list.push_back(static_cast<unsigned short>(j) + 1);
							break;
						}
					}
				}
			}

			return order_list_length_list;
		}


		unsigned short GetSequenceLength(unsigned short inSequenceIndex, const Editor::DriverInfo& inDriverInfo, const Emulation::IMemoryRandomReadAccess& inMemoryReader)
		{
			FOUNDATION_ASSERT(inSequenceIndex < 0x7f);

			const unsigned short sequence_0_address = inDriverInfo.GetMusicData().m_Sequence00Address;
			const unsigned short sequence_address = sequence_0_address + inSequenceIndex * inDriverInfo.GetMusicData().m_SequenceSize;

			for (unsigned short i = 0; i < inDriverInfo.GetMusicData().m_SequenceSize; ++i)
			{
				if (inMemoryReader[sequence_address + static_cast<unsigned short>(i)] == 0x7f)
					return i + 1;
			}

			return 0;
		}


		unsigned short GetEndOfMusicDataAddress(const Editor::DriverInfo& inDriverInfo, const Emulation::IMemoryRandomReadAccess& InMemoryReader)
		{
			FOUNDATION_ASSERT(inDriverInfo.IsValid());
			const unsigned char highest_sequence_index = GetHighestSequenceIndexUsed(inDriverInfo, InMemoryReader);

			// Get Music data descriptor
			const unsigned short sequence_data_address = inDriverInfo.GetMusicData().m_Sequence00Address + (inDriverInfo.GetMusicData().m_SequenceSize * (highest_sequence_index + 1));

			return sequence_data_address;
		}


		unsigned short GetEndOfFileAddress(const Editor::DriverInfo& inDriverInfo, const Emulation::IMemoryRandomReadAccess& inMemoryReader)
		{
			return GetEndOfMusicDataAddress(inDriverInfo, inMemoryReader);
		}
		

		std::vector<SIDWriteInformation> GetSIDWriteInformationFromDriver(Emulation::CPUMemory& inCPUMemory, const DriverInfo& inDriverInfo)
		{
			auto is_accessing_memory_address_with_offset = [](Emulation::CPUmos6510::AddressingMode inAddressingMode)
			{
				switch (inAddressingMode)
				{
				case Emulation::CPUmos6510::am_ABX:
				case Emulation::CPUmos6510::am_ABY:
					return true;
				default:
					break;
				}

				return false;
			};

			std::vector<SIDWriteInformation> result;
			std::unordered_map<unsigned char, SIDWriteInformation> sid_write_information;
		
			const int top_address = inDriverInfo.GetDescriptor().m_DriverCodeTop;
			const int bottom_address = top_address + inDriverInfo.GetDescriptor().m_DriverCodeSize;

			int address = top_address;

			bool first_write_found = false;
			unsigned char cycle_count = 0;

			inCPUMemory.Lock();

			while (address < bottom_address)
			{
				const unsigned char opcode = inCPUMemory[address];
				const unsigned char opcode_size = Emulation::CPUmos6510::GetOpcodeByteSize(opcode);
				const unsigned char opcode_cycles = Emulation::CPUmos6510::GetOpcodeCycles(opcode);
			
				const Emulation::CPUmos6510::AddressingMode opcode_addressing_mode = Emulation::CPUmos6510::GetOpcodeAddressingMode(opcode);

				if (is_accessing_memory_address_with_offset(opcode_addressing_mode))
				{
					FOUNDATION_ASSERT(opcode_size == 3);
;
					const unsigned short accessing_address = inCPUMemory.GetWord(address + 1);
					if (accessing_address >= 0xd400 && accessing_address <= 0xd406)
					{
						first_write_found = true;
						
						unsigned char sid_register = static_cast<unsigned char>(accessing_address & 0xff);
						
						auto it = sid_write_information.find(sid_register);
						if(it != sid_write_information.end())
							it->second.m_CycleOffset = cycle_count;
						else
							sid_write_information[sid_register] = { sid_register, cycle_count };
					}
				}

				address += static_cast<unsigned short>(opcode_size);

				if(first_write_found)
					cycle_count += opcode_cycles;
			}

			inCPUMemory.Unlock();

			// Push the collected information into the result vector
			for(const auto it : sid_write_information)
				result.push_back(it.second);

			// Sort writes in cycle order
			std::sort(result.begin(), result.end(), [](const auto& inA, const auto& inB) { return inA.m_CycleOffset < inB.m_CycleOffset; });

			// Adjust cycle offset to start from the the lowest (and first) entry in the list
			const unsigned char first_cycle = result.begin()->m_CycleOffset;
			for (auto& it : result)
				it.m_CycleOffset -= first_cycle;

			// Return the result
			return result;
		}


		unsigned int GetSongLengthInMilliseconds(Emulation::CPUMemory& inCPUMemory, const DriverInfo& inDriverInfo, unsigned char inSongIndex, bool inPAL, unsigned int inMaxMilliseconds)
		{
			const auto& driver_common = inDriverInfo.GetDriverCommon();
			const unsigned int track_count = inDriverInfo.GetMusicData().m_TrackCount;

			const unsigned int cycles_per_frame = inPAL ? EMULATION_CYCLES_PER_FRAME_PAL : EMULATION_CYCLES_PER_FRAME_NTSC;
			const unsigned int cycles_per_second = inPAL ? EMULATION_CYCLES_PER_SECOND_PAL : EMULATION_CYCLES_PER_SECOND_NTSC;

			auto frames_to_milliseconds = [&](unsigned long long inFrames)
			{
				return static_cast<unsigned int>(inFrames * cycles_per_frame * 1000ULL / cycles_per_second);
			};

			// Driver state after the order lists ran into an end mark without loop
			const unsigned char driver_state_stopped = 0x40;

			// The editor keeps playing on its own memory, the song plays on a copy
			std::vector<unsigned char> data(0x10000);

			inCPUMemory.Lock();
			inCPUMemory.GetData(0, data.data(), static_cast<unsigned int>(data.size()));
			inCPUMemory.Unlock();

			Emulation::CPUMemory memory(0x10000, &Utility::Global::instance().GetPlatform());
			Emulation::CPUmos6510 cpu;

			memory.Lock();
			memory.SetData(0, data.data(), static_cast<unsigned int>(data.size()));
			cpu.SetMemory(&memory);

			auto run = [&](unsigned short inAddress, unsigned char inAccumulator)
			{
				Emulation::CPUFrameCapture frame_capture(&cpu, 0xd400, 0xd400, cycles_per_frame);
				frame_capture.Capture(inAddress, inAccumulator);
			};

			// Frame n runs update n, its end is the play time
			run(driver_common.m_InitAddress, inSongIndex);

			// The order list index after a sequence fetch points behind the fetched entry. A track has looped
			// once a fetch lands on an index an earlier fetch had: a loop onto the last entry leaves the index
			// unchanged, the restart of the sequence shows the fetch
			const unsigned short order_list_index_address = driver_common.m_OrderListIndexAddress;
			const unsigned short sequence_index_address = driver_common.m_SequenceIndexAddress;

			std::vector<std::vector<bool>> fetched_index(track_count, std::vector<bool>(0x100, false));
			std::vector<unsigned char> last_order_list_index(track_count);
			std::vector<unsigned char> last_sequence_index(track_count);
			std::vector<bool> looped(track_count, false);

			for (unsigned int i = 0; i < track_count; ++i)
			{
				last_order_list_index[i] = memory[order_list_index_address + i];
				last_sequence_index[i] = memory[sequence_index_address + i];
				fetched_index[i][last_order_list_index[i]] = true;
			}

			unsigned int result = 0;

			for (unsigned long long frame = 1; frames_to_milliseconds(frame) <= inMaxMilliseconds; ++frame)
			{
				run(driver_common.m_UpdateAddress, 0);

				if (memory[driver_common.m_DriverStateAddress] == driver_state_stopped)
				{
					result = frames_to_milliseconds(frame);
					break;
				}

				bool all_looped = true;

				for (unsigned int i = 0; i < track_count; ++i)
				{
					const unsigned char order_list_index = memory[order_list_index_address + i];
					const unsigned char sequence_index = memory[sequence_index_address + i];

					const bool fetched = order_list_index != last_order_list_index[i] || sequence_index < last_sequence_index[i];

					if (fetched)
					{
						if (fetched_index[i][order_list_index])
							looped[i] = true;

						fetched_index[i][order_list_index] = true;
					}

					last_order_list_index[i] = order_list_index;
					last_sequence_index[i] = sequence_index;

					all_looped = all_looped && looped[i];
				}

				// The song is through once the last track starts over
				if (all_looped)
				{
					result = frames_to_milliseconds(frame);
					break;
				}
			}

			memory.Unlock();

			return result;
		}


		void InsertIRQ(const Editor::DriverInfo& inDriverInfo, Utility::C64FileWriter& inFileWriter)
		{
			unsigned char irq_assembly[] = {
				0xa9, 0x00, 0x20, 0x00, 0x10, 0x78, 0xa2, 0x00,
				0x8e, 0x0e, 0xdc, 0xe8, 0x8e, 0x1a, 0xd0, 0xa9,
				0x20, 0x8d, 0x14, 0x03, 0xa9, 0xc0, 0x8d, 0x15,
				0x03, 0xa9, 0x32, 0x8d, 0x12, 0xd0, 0x58, 0x60,
				0xa9, 0x1b, 0x8d, 0x11, 0xd0, 0xea, 0xea, 0xea,
				0xea, 0xea, 0xea, 0xee, 0x20, 0xd0, 0x20, 0x06,
				0x10, 0xce, 0x20, 0xd0, 0x6e, 0x19, 0xd0, 0x4c,
				0x31, 0xea
			};

			// Figure insertion location
			const unsigned short irq_vector = inFileWriter.GetWriteAddress();

			// Adjust driver vectors
			const unsigned short driver_init_vector = inDriverInfo.GetDriverCommon().m_InitAddress;
			const unsigned short driver_update_vector = inDriverInfo.GetDriverCommon().m_UpdateAddress;

			irq_assembly[0x03] = static_cast<unsigned char>(driver_init_vector & 0xff);
			irq_assembly[0x04] = static_cast<unsigned char>(driver_init_vector >> 8);
			irq_assembly[0x2f] = static_cast<unsigned char>(driver_update_vector & 0xff);
			irq_assembly[0x30] = static_cast<unsigned char>(driver_update_vector >> 8);

			// Adjust IRQ vectors
			const unsigned short irq_address_offset = 0x0020;
			const unsigned short irq_address = irq_vector + irq_address_offset;

			irq_assembly[0x10] = static_cast<unsigned char>(irq_address & 0xff);
			irq_assembly[0x15] = static_cast<unsigned char>(irq_address >> 8);

			// Write to file
			inFileWriter.WriteBytes(irq_assembly, sizeof(irq_assembly));
		}
	}
}
