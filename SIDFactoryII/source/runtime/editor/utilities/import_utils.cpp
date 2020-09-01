#include "runtime/editor/utilities/import_utils.h"
#include "runtime/editor/components_manager.h"
#include "runtime/editor/dialog/dialog_message.h"
#include "runtime/editor/driver/driver_utils.h"
#include "runtime/editor/driver/driver_info.h"
#include "runtime/editor/driver/idriver_architecture.h"
#include "runtime/editor/auxilarydata/auxilary_data_collection.h"

#include <map>
#include <assert.h>

namespace Editor
{
	namespace ImportUtils
	{
		std::map<unsigned char, unsigned char> GetTableMapping(const DriverInfo& inLoadedDriver, const DriverInfo& inImportDriver)
		{
			std::map<unsigned char, unsigned char> import_table_to_loaded_table_map;

			const std::vector<DriverInfo::TableDefinition>& loaded_table_definitions = inLoadedDriver.GetTableDefinitions();
			const std::vector<DriverInfo::TableDefinition>& import_table_definitions = inImportDriver.GetTableDefinitions();

			for (const auto& loaded_table : loaded_table_definitions)
			{
				const DriverInfo::TableDefinition* import_table = [&](const auto& loaded_table) -> const DriverInfo::TableDefinition*
				{
					for (const auto& import_table : import_table_definitions)
					{
						if (loaded_table.m_Name == import_table.m_Name)
							return &import_table;
					}

					return nullptr;
				}(loaded_table);

				if (import_table != nullptr)
					import_table_to_loaded_table_map[import_table->m_ID] = loaded_table.m_ID;
			}

			return import_table_to_loaded_table_map;
		}


		void ClearTableRowMajor(Emulation::CPUMemory& inCPUMemory, unsigned short inAddress, int inColumnCount, int inRowCount)
		{
			for (int row = 0; row < inRowCount; ++row)
			{
				for (int column = 0; column < inColumnCount; ++column)
				{
					unsigned short address = inAddress + row * inColumnCount + column;
					inCPUMemory[address] = 0;
				}
			}
		}


		void ClearTableColumnMajor(Emulation::CPUMemory& inCPUMemory, unsigned short inAddress, int inColumnCount, int inRowCount)
		{
			for (int row = 0; row < inRowCount; ++row)
			{
				for (int column = 0; column < inColumnCount; ++column)
				{
					unsigned short address = inAddress + column * inRowCount + row;
					inCPUMemory[address] = 0;
				}
			}
		}


		void ClearTables(const DriverInfo& inLoadedDriverInfo, Emulation::CPUMemory& inCPUMemory)
		{
			const std::vector<DriverInfo::TableDefinition> table_definitions = inLoadedDriverInfo.GetTableDefinitions();

			for (const auto& table_definition : table_definitions)
			{
				switch (table_definition.m_DataLayout)
				{
				case DriverInfo::TableDefinition::ColumnMajor:
					ClearTableColumnMajor(inCPUMemory, table_definition.m_Address, table_definition.m_ColumnCount, table_definition.m_RowCount);
					break;
				case DriverInfo::TableDefinition::RowMajor:
					ClearTableRowMajor(inCPUMemory, table_definition.m_Address, table_definition.m_ColumnCount, table_definition.m_RowCount);
					break;
				default:
					assert(false);
					break;
				}
			}
		}


		unsigned char ReadTableValueRowMajor(const Utility::C64File& inFile, const DriverInfo::TableDefinition& inTableDefinition, int inColumn, int inRow)
		{
			unsigned short address = inTableDefinition.m_Address + inRow * inTableDefinition.m_ColumnCount + inColumn;
			return inFile[address];
		}


		unsigned char ReadTableValueColumnMajor(const Utility::C64File& inFile, const DriverInfo::TableDefinition& inTableDefinition, int inColumn, int inRow)
		{
			unsigned short address = inTableDefinition.m_Address + inColumn * inTableDefinition.m_RowCount + inRow;
			return inFile[address];
		}


		void WriteTableValueRowMajor(Emulation::CPUMemory& inCPUMemory, unsigned char inValue, const DriverInfo::TableDefinition& inTableDefinition, int inColumn, int inRow)
		{
			unsigned short address = inTableDefinition.m_Address + inRow * inTableDefinition.m_ColumnCount + inColumn;
			inCPUMemory[address] = inValue;
		}


		void WriteTableValueColumnMajor(Emulation::CPUMemory& inCPUMemory, unsigned char inValue, const DriverInfo::TableDefinition& inTableDefinition, int inColumn, int inRow)
		{
			unsigned short address = inTableDefinition.m_Address + inColumn * inTableDefinition.m_RowCount + inRow;
			inCPUMemory[address] = inValue;
		}


		const DriverInfo::TableDefinition* GetTableDefinitionFromID(unsigned char inID, const std::vector<DriverInfo::TableDefinition>& inTableDefinitions)
		{
			for (const auto& table_definition : inTableDefinitions)
			{
				if (table_definition.m_ID == inID)
					return &table_definition;
			}

			return nullptr;
		}


		void CopyTable(const DriverInfo::TableDefinition& inSourceTableDefinition, const Utility::C64File& inSourceData, const DriverInfo::TableDefinition& inTargetTableDefinition, Emulation::CPUMemory& inCPUMemory)
		{
			int row_count = inSourceTableDefinition.m_RowCount < inTargetTableDefinition.m_RowCount ? inSourceTableDefinition.m_RowCount : inTargetTableDefinition.m_RowCount;
			int column_count = inSourceTableDefinition.m_ColumnCount < inTargetTableDefinition.m_ColumnCount ? inSourceTableDefinition.m_ColumnCount : inTargetTableDefinition.m_ColumnCount;

			for (int row = 0; row < row_count; ++row)
			{
				for (int column = 0; column < column_count; ++column)
				{
					unsigned char value = [&]()
					{
						switch (inSourceTableDefinition.m_DataLayout)
						{
						case DriverInfo::TableDefinition::RowMajor:
							return ReadTableValueRowMajor(inSourceData, inSourceTableDefinition, column, row);
						case DriverInfo::TableDefinition::ColumnMajor:
							return ReadTableValueColumnMajor(inSourceData, inSourceTableDefinition, column, row);
						default:
							assert(false);
							return static_cast<unsigned char>(0);
						}
					}();

					switch (inTargetTableDefinition.m_DataLayout)
					{
					case DriverInfo::TableDefinition::RowMajor:
						WriteTableValueRowMajor(inCPUMemory, value, inTargetTableDefinition, column, row);
						break;
					case DriverInfo::TableDefinition::ColumnMajor:
						WriteTableValueColumnMajor(inCPUMemory, value, inTargetTableDefinition, column, row);
						break;
					default:
						assert(false);
						break;
					}
				}
			}
		}

	
		void CopyTables(
			const std::map<unsigned char, 
			unsigned char>& table_mapping, 
			const DriverInfo& inLoadedDriverInfo, 
			Emulation::CPUMemory& inCPUMemory, 
			const DriverInfo& inImportDriverInfo, 
			const Utility::C64File& inImportFile
		)
		{
			const std::vector<DriverInfo::TableDefinition> import_table_definitions = inImportDriverInfo.GetTableDefinitions();

			for (const auto& source_table_definition : import_table_definitions)
			{
				auto it = table_mapping.find(source_table_definition.m_ID);
				if (it != table_mapping.end())
				{
					unsigned char id = it->second;
					const DriverInfo::TableDefinition* target_table_definition = GetTableDefinitionFromID(id, inLoadedDriverInfo.GetTableDefinitions());

					if (target_table_definition != nullptr)
						CopyTable(source_table_definition, inImportFile, *target_table_definition, inCPUMemory);
				}
			}
		}

	
		void ClearMusicData(const DriverInfo& inLoadedDriverInfo, Emulation::CPUMemory& inCPUMemory)
		{
			const auto& music_data = inLoadedDriverInfo.GetMusicData();
			Editor::IDriverArchitecture* driver_architecture = inLoadedDriverInfo.GetDriverArchitecture();

			// Clear tracks
			for (int i = 0; i < music_data.m_TrackCount; ++i)
			{
				unsigned short orderlist_address = music_data.m_OrderListTrack1Address + music_data.m_OrderListSize * i;

				for (int j = 0; j < music_data.m_OrderListSize; ++j)
					inCPUMemory[orderlist_address + j] = 0;

				driver_architecture->SetEmptyOrderList(&inCPUMemory, orderlist_address);
			}

			// Clear sequences
			for (int i = 0; i < music_data.m_SequenceCount; ++i)
			{
				unsigned short sequence_address = music_data.m_Sequence00Address + music_data.m_SequenceSize * i;

				for (int j = 0; j < music_data.m_SequenceSize; ++j)
					inCPUMemory[sequence_address + j] = 0;

				driver_architecture->SetEmptySequence(&inCPUMemory, sequence_address);
			}
		}

	
		void CopyMusicData(const DriverInfo& inLoadedDriverInfo, Emulation::CPUMemory& inCPUMemory, const DriverInfo& inImportDriverInfo, const Utility::C64File& inImportFile)
		{
			unsigned char highest_sequence_index_used = DriverUtils::GetHighestSequenceIndexUsed(inImportDriverInfo, inImportFile);

			const auto& import_music_data = inImportDriverInfo.GetMusicData();
			const auto& loaded_music_data = inLoadedDriverInfo.GetMusicData();

			// Copy order lists
			int track_count = import_music_data.m_TrackCount < loaded_music_data.m_TrackCount ? import_music_data.m_TrackCount : loaded_music_data.m_TrackCount;
			int orderlist_size = import_music_data.m_OrderListSize < loaded_music_data.m_OrderListSize ? import_music_data.m_OrderListSize : loaded_music_data.m_OrderListSize;

			for (int i = 0; i < track_count; ++i)
			{
				unsigned short import_address = import_music_data.m_OrderListTrack1Address + import_music_data.m_OrderListSize * i;
				unsigned short target_address = loaded_music_data.m_OrderListTrack1Address + loaded_music_data.m_OrderListSize * i;

				for (int j = 0; j < orderlist_size; ++j)
					inCPUMemory[target_address + j] = inImportFile[import_address + j];
			}

			// Copy sequences
			int sequence_size = import_music_data.m_SequenceSize < loaded_music_data.m_SequenceSize ? import_music_data.m_SequenceSize : loaded_music_data.m_SequenceSize;

			for (int i = 0; i <= highest_sequence_index_used; ++i)
			{
				unsigned short import_address = import_music_data.m_Sequence00Address + import_music_data.m_SequenceSize * i;
				unsigned short target_address = loaded_music_data.m_Sequence00Address + loaded_music_data.m_SequenceSize * i;

				for (int j = 0; j < sequence_size; ++j)
				{
					unsigned short current_import_address = import_address + j;

					if (current_import_address < inImportFile.GetBottomAddress())
						inCPUMemory[target_address + j] = inImportFile[current_import_address];
					else
						inCPUMemory[target_address + j] = 0;
				}
			}
		}


		void Import(
			ScreenBase* inCallerScreen,
			DriverInfo& inLoadedDriverInfo,
			Emulation::CPUMemory& inCPUMemory,
			const DriverInfo& inImportDriverInfo,
			const Utility::C64File& inImportFile
		)
		{
			// Map tables
			std::map<unsigned char, unsigned char> table_mapping = GetTableMapping(inLoadedDriverInfo, inImportDriverInfo);

			// Unlock CPU memory for writing
			inCPUMemory.Lock();

			// Clear tables
			ClearTables(inLoadedDriverInfo, inCPUMemory);

			// Copy tables
			CopyTables(table_mapping, inLoadedDriverInfo, inCPUMemory, inImportDriverInfo, inImportFile);

			// Clear music data
			ClearMusicData(inLoadedDriverInfo, inCPUMemory);

			// Copy music data
			CopyMusicData(inLoadedDriverInfo, inCPUMemory, inImportDriverInfo, inImportFile);

			// Copy auxilary data
			inLoadedDriverInfo.GetAuxilaryDataCollection() = inImportDriverInfo.GetAuxilaryDataCollection();

			// Lock CPU memory for writing
			inCPUMemory.Unlock();
		}
	}
}
