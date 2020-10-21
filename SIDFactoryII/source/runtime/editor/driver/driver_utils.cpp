#include "runtime/editor/driver/driver_utils.h"
#include "runtime/editor/datasources/datasource_table.h"
#include "runtime/editor/datasources/datasource_table_column_major.h"
#include "runtime/editor/datasources/datasource_table_row_major.h"
#include "runtime/editor/driver/driver_info.h"
#include "runtime/emulation/cpumemory.h"
#include "runtime/emulation/imemoryrandomreadaccess.h"
#include "utils/c64file.h"
#include <assert.h>

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
				// Get Music data descriptor
				const DriverInfo::MusicData& music_data = inDriverInfo.GetMusicData();

				// Find highest used sequence index
				unsigned short order_list_1 = music_data.m_OrderListTrack1Address;

				for (unsigned char i = 0; i < music_data.m_TrackCount; ++i)
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
				std::vector<int> usage_count(inDriverInfo.GetMusicData().m_SequenceCount, 0);

				// Get Music data descriptor
				const DriverInfo::MusicData& music_data = inDriverInfo.GetMusicData();

				// Find highest used sequence index
				unsigned short order_list_1 = music_data.m_OrderListTrack1Address;

				for (unsigned char i = 0; i < music_data.m_TrackCount; ++i)
				{
					unsigned short order_list_address = order_list_1 + static_cast<unsigned short>(i) * music_data.m_OrderListSize;
					for (unsigned short j = 0; j < music_data.m_OrderListSize; ++j)
					{
						unsigned char value = inMemoryReader[order_list_address + j];
						if (value < 0x80)
						{
							if (value == 0x7f)
								break;

							usage_count[value]++;
						}
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

			assert(highest_used_index < 0x100);
			return static_cast<unsigned char>(highest_used_index);
		}


		std::vector<unsigned short> GetOrderListsLength(const Editor::DriverInfo& inDriverInfo, const Emulation::IMemoryRandomReadAccess& inMemoryReader)
		{
			std::vector<unsigned short> order_list_length_list;

			if (inDriverInfo.HasParsedHeaderBlock(DriverInfo::HeaderBlockID::ID_MusicData))
			{
				// Get Music data descriptor
				const DriverInfo::MusicData& music_data = inDriverInfo.GetMusicData();

				// Find highest used sequence index
				unsigned short order_list_1 = music_data.m_OrderListTrack1Address;

				for (unsigned char i = 0; i < music_data.m_TrackCount; ++i)
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
					}
				}
			}

			return order_list_length_list;
		}


		unsigned short GetSequenceLength(unsigned short inSequenceIndex, const Editor::DriverInfo& inDriverInfo, const Emulation::IMemoryRandomReadAccess& inMemoryReader)
		{
			assert(inSequenceIndex < 0x7f);

			const unsigned short sequence_0_address = inDriverInfo.GetMusicData().m_Sequence00Address;
			const unsigned short sequence_address = sequence_0_address + inSequenceIndex * inDriverInfo.GetMusicData().m_SequenceSize;

			for (unsigned char i = 0; i < inDriverInfo.GetMusicData().m_SequenceSize; ++i)
			{
				if (inMemoryReader[sequence_address + static_cast<unsigned short>(i)] == 0x7f)
					return i + 1;
			}

			return 0;
		}


		unsigned short GetEndOfMusicDataAddress(const Editor::DriverInfo& inDriverInfo, const Emulation::IMemoryRandomReadAccess& InMemoryReader)
		{
			assert(inDriverInfo.IsValid());
			const unsigned char highest_sequence_index = GetHighestSequenceIndexUsed(inDriverInfo, InMemoryReader);

			// Get Music data descriptor
			const unsigned short sequence_data_address = inDriverInfo.GetMusicData().m_Sequence00Address + (inDriverInfo.GetMusicData().m_SequenceSize * (highest_sequence_index + 1));

			return sequence_data_address;
		}


		unsigned short GetEndOfFileAddress(const Editor::DriverInfo& inDriverInfo, const Emulation::IMemoryRandomReadAccess& inMemoryReader)
		{
			return GetEndOfMusicDataAddress(inDriverInfo, inMemoryReader);
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
