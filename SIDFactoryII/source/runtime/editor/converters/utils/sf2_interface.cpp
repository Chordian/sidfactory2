#include "sf2_interface.h"
#include "runtime/emulation/cpumemory.h"
#include "runtime/editor/auxilarydata/auxilary_data_collection.h"
#include "runtime/editor/auxilarydata/auxilary_data_hardware_preferences.h"
#include "runtime/editor/screens/screen_edit_utils.h"
#include "runtime/editor/datasources/datasource_orderlist.h"
#include "runtime/editor/datasources/datasource_sequence.h"
#include "runtime/editor/datasources/datasource_table.h"
#include "runtime/editor/datasources/datasource_table_text.h"
#include "runtime/editor/driver/driver_utils.h"
#include "runtime/editor/driver/driver_info.h"
#include "foundation/platform/iplatform.h"
#include "utils/utilities.h"
#include "utils/c64file.h"

#include <assert.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#if __has_include(<filesystem>)
#include <filesystem>
#endif
#include <vector>
#include <algorithm>

using namespace Foundation;
using namespace Emulation;
using namespace Editor;

// @todo This is temporary; see GetTableData() below.
#define TABLENAME_WAVE		"Wave"
#define TABLENAME_PULSE		"Pulse"
#define TABLENAME_FILTER	"Filter"
#define TABLENAME_HR		"HR"
#define TABLENAME_ARPEGGIO	"Arp"
#define TABLENAME_TEMPO		"Tempo"
#define TABLENAME_INIT		"Init"

namespace SF2
{
	Interface::Interface(IPlatform* inPlatform, Editor::ComponentConsole& inConsole)
		: m_Platform(inPlatform)
		, m_Range({ 0, 0 })
	{
		m_StreamOutputBuffer = Editor::ConsoleOStreamBuffer(&inConsole);
		m_COutStream = std::make_shared<COutStream>(&m_StreamOutputBuffer);

		m_EntireBlock = new unsigned char[0x10000];
		m_CPUMemory = new CPUMemory(0x10000, m_Platform);

		m_DriverInfo = std::make_shared<DriverInfo>();

		m_CommandName[Cmd_Slide]		= "Slide";
		m_CommandName[Cmd_Vibrato]		= "Vibrato";
		m_CommandName[Cmd_Portamento]	= "Portamento";
		m_CommandName[Cmd_Arpeggio]		= "Arpeggio/Chord";
		m_CommandName[Cmd_Fret]			= "Fret";
		m_CommandName[Cmd_ADSR_Note]	= "ADSR (Note)";
		m_CommandName[Cmd_ADSR_Persist]	= "ADSR (Persistant)";
		m_CommandName[Cmd_Index_Filter]	= "Filter Index";
		m_CommandName[Cmd_Index_Wave]	= "Wave Index";
		m_CommandName[Cmd_Demo_Flag]	= "Demo Flag";

		m_CommandName[Cmd_Index_Pulse]	= "Pulse Index";
		m_CommandName[Cmd_Tempo]		= "Tempo";
		m_CommandName[Cmd_Volume]		= "Main Volume";
	}

	Interface::~Interface()
	{
		delete m_EntireBlock;
		delete m_CPUMemory;
	}


	std::ostream& Interface::GetCout()
	{
		return *m_COutStream;
	}


	/**
	 * Load the SF2 driver into the emulated C64 memory.
	 *
	 * @origin	runtime/editor/editor_facility.cpp
	 */
	bool Interface::LoadFile(const std::string& inPathAndFilename)
	{
		void* data = nullptr;
		long data_size = 0;

		std::shared_ptr<DriverInfo> driver_info = std::make_shared<DriverInfo>();

		if (Utility::ReadFile(inPathAndFilename, 65536, &data, data_size))
		{
			if (data_size > 2)
			{
				std::shared_ptr<Utility::C64File> c64_file = Utility::C64File::CreateFromPRGData(data, data_size);
				driver_info->Parse(*c64_file);

				if (driver_info->IsValid())
				{
					m_DriverInfo = driver_info;
					m_DriverInfo->GetAuxilaryDataCollection().Reset();

					const unsigned short auxilary_data_vector = m_DriverInfo->GetDriverCommon().m_InitAddress - 5;
					const unsigned short auxilary_data_address = c64_file->GetWord(auxilary_data_vector);

					if (auxilary_data_address != 0)
					{
						Utility::C64FileReader reader = Utility::C64FileReader(*c64_file, auxilary_data_address);
						m_DriverInfo->GetAuxilaryDataCollection().Load(reader);
					}

					ParseDriverDetails();

					// Copy the data to the emulated memory
					m_CPUMemory->Lock();
					m_CPUMemory->Clear();
					m_CPUMemory->SetData(c64_file->GetTopAddress(), c64_file->GetData(), c64_file->GetDataSize());
					m_CPUMemory->Unlock();

					InitData();
				}
			}

			delete[] static_cast<char*>(data);
		}

		return driver_info->IsValid();
	}

	/**
	 * Save the SF2 data to a source file ready to be loaded into the editor.
	 *
	 * @origin	runtime/editor/editor_facility.cpp
	 */
	std::shared_ptr<Utility::C64File> Interface::GetResult()
	{
		if (m_DriverInfo->IsValid())
		{
			m_CPUMemory->Lock();

			unsigned short top_of_file_address = m_DriverInfo->GetTopAddress();
			unsigned short end_of_file_address = DriverUtils::GetEndOfMusicDataAddress(*m_DriverInfo, reinterpret_cast<const Emulation::IMemoryRandomReadAccess&>(*m_CPUMemory));
			unsigned short data_size = end_of_file_address - top_of_file_address;

			unsigned char* data = new unsigned char[data_size];
			m_CPUMemory->GetData(top_of_file_address, data, data_size);
			m_CPUMemory->Unlock();
			std::shared_ptr<Utility::C64File> file = Utility::C64File::CreateFromData(top_of_file_address, data, data_size);
			delete[] data;

			Utility::C64FileWriter file_writer(*file, end_of_file_address, true);

			unsigned short irq_vector = file_writer.GetWriteAddress();
			DriverUtils::InsertIRQ(*m_DriverInfo, file_writer);

			unsigned short auxilary_data_vector = file_writer.GetWriteAddress();
			m_DriverInfo->GetAuxilaryDataCollection().Save(file_writer);

			// Adjust IRQ and auxilary data vectors in file
			const unsigned short driver_init_vector = m_DriverInfo->GetDriverCommon().m_InitAddress;
			(*file)[driver_init_vector - 2] = static_cast<unsigned char>(irq_vector & 0xff);
			(*file)[driver_init_vector - 1] = static_cast<unsigned char>(irq_vector >> 8);
			(*file)[driver_init_vector - 5] = static_cast<unsigned char>(auxilary_data_vector & 0xff);
			(*file)[driver_init_vector - 4] = static_cast<unsigned char>(auxilary_data_vector >> 8);

			return file;
		}

		return nullptr;
	}

	/**
	 * Return a pointer to the entire 64 KB of the emulated C64 memory.
	 */
	unsigned char* Interface::GetAllMemory() const
	{
		assert(m_CPUMemory != nullptr);

		if (m_DriverInfo->IsValid())
		{
			m_CPUMemory->Lock();
			m_CPUMemory->GetData(0, m_EntireBlock, 0x10000);
			m_CPUMemory->Unlock();
		}

		return m_EntireBlock;
	}

	/**
	 * Return the start and end address of the SF2 file data.
	 */
	Interface::Range Interface::GetRangeSF2()
	{
		assert(m_CPUMemory != nullptr);

		if (m_DriverInfo->IsValid())
		{
			m_CPUMemory->Lock();

			m_Range.m_StartAddress = m_DriverInfo->GetTopAddress();
			m_Range.m_EndAddress = DriverUtils::GetEndOfMusicDataAddress(*m_DriverInfo, reinterpret_cast<const Emulation::IMemoryRandomReadAccess&>(*m_CPUMemory));

			m_CPUMemory->Unlock();
		}

		return m_Range;
	}

	/**
	 * Initialize the SF2 order lists and sequences.
	 *
	 * @origin	runtime/editor/screens/screen_edit.cpp
	 */
	void Interface::InitData()
	{
		assert(m_DriverInfo != nullptr);
		assert(m_CPUMemory != nullptr);

		m_CPUMemory->Lock();
		ScreenEditUtils::PrepareSequenceData(*m_DriverInfo, *m_CPUMemory);
		ScreenEditUtils::PrepareSequencePointers(*m_DriverInfo, *m_CPUMemory);
		m_CPUMemory->Unlock();

		// Create data containers for each track
		ScreenEditUtils::PrepareOrderListsDataSources(*m_DriverInfo, *m_CPUMemory, m_OrderListDataSources);

		// Create data containers for each sequence
		ScreenEditUtils::PrepareSequenceDataSources(*m_DriverInfo, m_DriverState, *m_CPUMemory, m_SequenceDataSources);

		// Replace first transpose byte 0xa0 with 0xff to solidify this is a true virgin state
		// NOTE: This is only in effect during converting; it is restored when pushing data if it still persists.
		for (int i = 0; i < m_DriverDetails.m_TrackCount; i++)
		{
			std::shared_ptr<DataSourceOrderList>& orderlist_track = m_OrderListDataSources[i];
			(*orderlist_track)[0] = { 0xff, 0x00 };
		}

		// Set all sequence sizes to 0 (the small one event sequences are obstructing the conversions)
		// NOTE: This is only for appending during converting; the sequence sizes are restored when pushing data.
		for (int i = 0; i < m_DriverDetails.m_SequenceCount; i++)
		{
			const std::shared_ptr<DataSourceSequence>& sequence_source = m_SequenceDataSources[i];
			sequence_source->SetLength(0);
		}
	}

	/**
	 * Set up a struct with driver info details.
	 *
	 * This is a mix of info from 'driver_info.cpp' or set up here after detecting
	 * driver version (thus some of the driver info is converter only).
	 */
	void Interface::ParseDriverDetails()
	{
		m_DriverDetails.m_DriverName = m_DriverInfo->GetDescriptor().m_DriverName;
		m_DriverDetails.m_DriverVersionMajor = m_DriverInfo->GetDescriptor().m_DriverVersionMajor;
		m_DriverDetails.m_DriverVersionMinor = m_DriverInfo->GetDescriptor().m_DriverVersionMinor;

		m_DriverDetails.m_TrackCount = m_DriverInfo->GetMusicData().m_TrackCount;
		m_DriverDetails.m_SequenceCount = m_DriverInfo->GetMusicData().m_SequenceCount;

		for (int table_type = 0; table_type < TableType::_MAX; table_type++)
		{
			TableData table_data = GetTableData(table_type);
			// Set column count to 0 if the driver doesn't have this table
			m_DriverDetails.m_TableColCount[table_type] = table_data.m_DataSourceTable == nullptr ? 0 : table_data.m_TableDefinition.m_ColumnCount;
		}

		switch (m_DriverDetails.m_DriverVersionMajor)
		{
		case 11: // Driver 11.02
			m_SupportedCommands = {
				Cmd_Slide,
				Cmd_Vibrato,
				Cmd_Portamento,
				Cmd_Arpeggio,
				Cmd_ADSR_Note,
				Cmd_ADSR_Persist,
				Cmd_Index_Filter,
				Cmd_Index_Wave,
				Cmd_Index_Pulse,
				Cmd_Tempo,
				Cmd_Volume,
				Cmd_Demo_Flag
			};
			m_CommandFormat = { /* Byte position */ 0, /* Byte mask */ 0xff };
			
			for (int table_type = 0; table_type < TableType::_MAX; table_type++)
				switch (table_type)
				{
				case TableType::Wave:
					m_DriverDetails.m_TableWrap[table_type] = { 0x00, 0xff, { 0x7f }, 0x01, 0xff };
					break;

				case TableType::Pulse:
				case TableType::Filter:
					m_DriverDetails.m_TableWrap[table_type] = { 0x00, 0xff, { 0x7f }, 0x02, 0xff };
					break;

				case TableType::Tempo:
					m_DriverDetails.m_TableWrap[table_type] = { 0x00, 0xff, { 0x7f }, -1, 0xff };
					break;

				case TableType::Arpeggio:
					// Doesn't matter as arpeggio wrap values are always relative in this driver
				default:
					m_DriverDetails.m_TableWrap[table_type] = { /* No byte ID position */ -1, /* Byte ID mask */ 0xff, /* Byte ID values */ { 0x7f }, /* No wrap value position */ -1, /* Wrap value mask */ 0xff };
				}

			break;

		default:
			break;
		}
	}

	/**
	 * Returns a struct for a table describing how wrap markers work.
	 */
	Interface::WrapFormat Interface::GetWrapFormat(int inTableType)
	{
		if (!IsTableSupported(inTableType, true))
			return WrapFormat();

		return m_DriverDetails.m_TableWrap[inTableType];
	}

	/**
	 * Returns a bool whether the specified table is supported by the SF2 driver.
	 *
	 * If TRUE is specified as the second paramater, a message is written to the
	 * console and the program stops.
	 */
	bool Interface::IsTableSupported(int inTableType, bool inCoutOnError)
	{
		int column_count = m_DriverDetails.m_TableColCount[inTableType];
		if (inCoutOnError && column_count == 0)
		{
			GetCout() << "\nERROR: The specified table is not defined in this driver." << std::endl;
		}

		return column_count != 0;
	}

	/**
	 * Returns a bool whether the command table in the SF2 driver supports this command.
	 *
	 * If TRUE is specified as the second parameter, a message is written to the
	 * console as a warning. The program is not stopped.
	 */
	bool Interface::IsCommandSupported(unsigned char inCommand, bool inCoutOnWarning)
	{
		unsigned char command = inCommand & m_CommandFormat.m_ByteMask;

		bool command_supported = find(m_SupportedCommands.begin(), m_SupportedCommands.end(), command) != m_SupportedCommands.end();
		if (inCoutOnWarning && !command_supported)
		{
			for (unsigned char checked_command : m_CommandChecked)
				if (inCommand == checked_command) return false;
			GetCout() << "WARNING: This driver does not have a \"" << m_CommandName[command] << "\" command." << std::endl;
			m_CommandChecked.push_back(inCommand);
		}

		return command_supported;
	}

	/**
	 * Return the long name of the SF2 driver used as conversion target.
	 */
	std::string Interface::GetDriverName()
	{
		return m_DriverDetails.m_DriverName;
	}

	/**
	 * Return the name of the specified command as used in the dedicated table.
	 */
	std::string Interface::GetCommandName(unsigned char inCommand)
	{
		return m_CommandName[inCommand & m_CommandFormat.m_ByteMask];
	}

	/**
	 * Return an order list container for a track.
	 */
	std::vector<unsigned char> Interface::GetContainerOrderList(int inTrack)
	{
		std::vector<unsigned char> order_list;
		order_list.push_back(inTrack);

		std::shared_ptr<DataSourceOrderList>& orderlist_track = m_OrderListDataSources[inTrack];

		for (unsigned int i = 0; i < orderlist_track->GetLength(); i++) {
			const DataSourceOrderList::Entry orderlist_entry = (*orderlist_track)[i];
			order_list.push_back(orderlist_entry.m_Transposition);
			order_list.push_back(orderlist_entry.m_SequenceIndex);
		}

		return order_list;
	}

	/**
	 * Add an entry (transpose and sequence index) at the end of the order list.
	 *
	 * @origin	runtime/editor/components/component_track.cpp
	 */
	bool Interface::AppendToOrderList(int inTrack, std::vector<unsigned char> inBytes)
	{
		assert(inBytes.size() == 2);

		unsigned char transpose = inBytes[0], sequence_index = inBytes[1];

		assert(inTrack < m_DriverDetails.m_TrackCount);
		assert(sequence_index < 0x80);

		const std::shared_ptr<DataSourceOrderList>& orderlist_track = m_OrderListDataSources[inTrack];
		const DataSourceOrderList::Entry orderlist_entry = { transpose, sequence_index };

		if (orderlist_track->CanIncreaseSize())
		{
			if (orderlist_track->GetLength() == 2 && (*orderlist_track)[0].m_Transposition == 0xff && (*orderlist_track)[0].m_SequenceIndex == 0x00)
				// It's the first (adapted) virgin position (ff 00) so just overwrite that
				(*orderlist_track)[0] = orderlist_entry;
			else
			{
				orderlist_track->IncreaseSize();
				const unsigned int orderlist_length = orderlist_track->GetLength();

				(*orderlist_track)[orderlist_length - 1] = (*orderlist_track)[orderlist_length - 2]; // Move ff 00 out
				(*orderlist_track)[orderlist_length - 2] = orderlist_entry;
			}
			return true;
		}

		return false;
	}

	/**
	 * Add an entry (instrument, command and note) at the end of the sequence.
	 *
	 * @origin	runtime/editor/components/component_track.cpp
	 */
	bool Interface::AppendToSequence(int inSequenceIndex, std::vector<unsigned char> inBytes)
	{
		assert(inBytes.size() == 3);

		unsigned char instrument = inBytes[0], command = inBytes[1], note = inBytes[2];

		assert(instrument < 0x20 || instrument == 0x80 || instrument == 0x90);
		assert(command < 0x40 || command == 0x80);
		assert(note < 0x70 || note == 0x7e);

		const std::shared_ptr<DataSourceSequence>& sequence_source = m_SequenceDataSources[inSequenceIndex];
		const DataSourceSequence::Event sequence_event = { (unsigned char)(instrument < 0x20 ? instrument + 0xa0 : instrument), (unsigned char)(command < 0x40 ? command + 0xc0 : command), note };

		const unsigned int sequence_length = sequence_source->GetLength();

		if (sequence_length < DataSourceSequence::MaxEventCount) {
			sequence_source->SetLength(sequence_length + 1);

			(*sequence_source)[sequence_length] = sequence_event;

			return true;
		}

		return false;
	}

	/**
	 * Pack and push all order lists and sequences to the emulated C64 memory.
	 *
	 * NOTE: Although modifications to tables are pushed immediately elsewhere, all
	 * wrap markers can be kept relative then changed to absolute if needed here.
	 *
	 * @origin	runtime/editor/components/component_track.cpp
	 */
	bool Interface::PushAllDataToMemory(bool inFixRelativeMarkers)
	{
		for (int i = 0; i < m_DriverDetails.m_TrackCount; i++) {

			// Order lists
			std::shared_ptr<DataSourceOrderList>& orderlist_track = m_OrderListDataSources[i];

			// If still a modified virgin state (ff 00) reset it back to true default (a0 00)
			if (orderlist_track->GetLength() == 2 && (*orderlist_track)[0].m_Transposition == 0xff && (*orderlist_track)[0].m_SequenceIndex == 0x00)
				(*orderlist_track)[0] = { 0xa0, 0x00 };

			DataSourceOrderList::PackResult packed_result = orderlist_track->Pack();
			if (packed_result.m_DataLength >= 0x100)
				return false;

			// Put packed orderlist in C64 memory
			orderlist_track->SendPackedDataToBuffer(packed_result);
			m_CPUMemory->Lock();
			orderlist_track->PushDataToSource();
			m_CPUMemory->Unlock();
		}

		bool length_modified;

		for (int i = 0; i < m_DriverDetails.m_SequenceCount; i++) {
			const std::shared_ptr<DataSourceSequence>& sequence_source = m_SequenceDataSources[i];

			// Set all untouched sequence sizes at 0 back to 1 for the packer (and editor) to work
			length_modified = false;
			if (sequence_source->GetLength() == 0)
			{
				sequence_source->SetLength(1);
				length_modified = true;
			}

			DataSourceSequence::PackResult packed_result = sequence_source->Pack();
			if (packed_result.m_DataLength >= 0x100 || packed_result.m_Data == nullptr)
				return false;
			sequence_source->SendPackedDataToBuffer(packed_result);
			m_CPUMemory->Lock();
			sequence_source->PushDataToSource();
			m_CPUMemory->Unlock();

			// Go back to 0 size sequences again in case more work needs to be done
			if (length_modified)
				sequence_source->SetLength(0);
		}

		if (inFixRelativeMarkers)
		{
			// Change relative wrap markers in tables to absolute where needed
			for (int table_type = 0; table_type < TableType::_MAX; table_type++)
			{
				Table table = GetTable(table_type);
				TableData table_data = GetTableData(table_type);
				WrapFormat wrap_info = m_DriverDetails.m_TableWrap[table_type];

				// Does this table use wrap markers with an index?
				if (wrap_info.m_ByteWrapPosition != -1)
				{
					int cluster_row = 0;
					for (int row = 0; row < table.m_RowCount; row++)
					{
						// Loop possible wrap ID byte values (e.g. 0x7e, 0x7f, etc.)
						for (auto wrap_id : wrap_info.m_ByteID)
							// Is this a wrap ID byte?
							if ((*table_data.m_DataSourceTable)[(row * table.m_ColumnCount) + wrap_info.m_ByteIDPosition] == wrap_id)
							{
								// Wrap ID byte found; adapt the wrap value
								unsigned char wrap_byte_pos = (row * table.m_ColumnCount) + wrap_info.m_ByteWrapPosition;
								(*table_data.m_DataSourceTable)[wrap_byte_pos] = (((*table_data.m_DataSourceTable)[wrap_byte_pos] & wrap_info.m_ByteWrapMask) + cluster_row) & wrap_info.m_ByteWrapMask;
								cluster_row = row + 1;

								PushTableToSource(table_data);
								break;
							}
					}
				}
			}
		}
		return true;
	}

	/**
	 * Push all tables to the emulated C64 memory.
	 */
	void Interface::PushTableToSource(Interface::TableData inDataSourceTable)
	{
		m_CPUMemory->Lock();
		inDataSourceTable.m_DataSourceTable->PushDataToSource();
		m_CPUMemory->Unlock();
	}

	/**
	 * Return the block in the C64 memory with the order list for the track.
	 */
	Interface::Block Interface::GetRangeOrderList(int inTrack)
	{
		std::stringstream description;
		description << "Track " << inTrack << " order list in C64 memory";

		auto music_data = m_DriverInfo->GetMusicData();
		return { description.str(), music_data.m_OrderListTrack1Address + ((unsigned int)music_data.m_OrderListSize * inTrack), GetAllMemory(), (unsigned int)music_data.m_OrderListSize };
	}

	/**
	 * Return the block in the C64 memory with the specified sequence.
	 */
	Interface::Block Interface::GetRangeSequence(unsigned int inSequenceIndex)
	{
		auto music_data = m_DriverInfo->GetMusicData();

		std::stringstream description;
		description << "Sequence #" << std::setfill('0') << std::hex << std::setw(2) << (int)inSequenceIndex << " in C64 memory";

		return { description.str(), music_data.m_Sequence00Address + (music_data.m_SequenceSize * inSequenceIndex), GetAllMemory(), (unsigned int)music_data.m_SequenceSize };
	}

	/**
	 * Return a sequence container.
	 */
	std::vector<unsigned char> Interface::GetContainerSequence(int inSequenceIndex)
	{
		std::vector<unsigned char> sequence;
		sequence.push_back(inSequenceIndex);

		std::shared_ptr<DataSourceSequence>& sequence_source = m_SequenceDataSources[inSequenceIndex];

		for (unsigned int i = 0; i < sequence_source->GetLength(); i++) {
			const DataSourceSequence::Event sequence_event = (*sequence_source)[i];
			sequence.push_back(sequence_event.m_Instrument);
			sequence.push_back(sequence_event.m_Command);
			sequence.push_back(sequence_event.m_Note);
		}

		return sequence;
	}

	/**
	 * Get the data source and definition for a specific type of table.
	 */
	Interface::TableData Interface::GetTableData(int inTableType)
	{
		TableData table_data;
		table_data.m_DataSourceTable = nullptr;

		const std::vector<DriverInfo::TableDefinition>& table_definitions = m_DriverInfo->GetTableDefinitions();

		for (const auto& table_definition : table_definitions)
		{
			// @todo Replace defines with additional table types when added by Laxity later
			if ((inTableType == TableType::Instruments	&& table_definition.m_Type == DriverInfo::TableType::Instruments) ||
				(inTableType == TableType::Commands		&& table_definition.m_Type == DriverInfo::TableType::Commands) ||
				(inTableType == TableType::Wave			&& table_definition.m_Name == TABLENAME_WAVE) ||
				(inTableType == TableType::Pulse		&& table_definition.m_Name == TABLENAME_PULSE) ||
				(inTableType == TableType::Filter		&& table_definition.m_Name == TABLENAME_FILTER) ||
				(inTableType == TableType::HR			&& table_definition.m_Name == TABLENAME_HR) ||
				(inTableType == TableType::Arpeggio		&& table_definition.m_Name == TABLENAME_ARPEGGIO) ||
				(inTableType == TableType::Tempo		&& table_definition.m_Name == TABLENAME_TEMPO) ||
				(inTableType == TableType::Init			&& table_definition.m_Name == TABLENAME_INIT))
			{
				table_data.m_DataSourceTable = DriverUtils::CreateTableDataSource(table_definition, m_CPUMemory);
				table_data.m_TableDefinition = table_definition;
				break;
			}
		}
		return table_data;
	}

	/**
	 * Return the contents of a specific data source table.
	 *
	 * @origin	runtime/editor/screens/screen_edit.cpp
	 */
	Interface::Table Interface::GetTable(int inTableType)
	{
		if (!IsTableSupported(inTableType, true))
			return Table();

		Table table;
		TableData table_data = GetTableData(inTableType);

		table.m_Name = table_data.m_TableDefinition.m_Name;
		table.m_ColumnCount = table_data.m_DataSourceTable->GetColumnCount();
		table.m_RowCount = GetLastUnusedRow(table_data.m_DataSourceTable);

		for (int i = 0; i < table.m_ColumnCount * table.m_RowCount; i++)
			table.m_Data.push_back((*table_data.m_DataSourceTable)[i]);

		return table;
	}

	/**
	 * Return the last row number with actual table data.
	 */
	const int Interface::GetLastUnusedRow(std::shared_ptr<DataSourceTable> inDataSourceTable) const
	{
		int max_rows = inDataSourceTable->GetRowCount(),
			max_cols = inDataSourceTable->GetColumnCount();

		for (int i = (max_rows * max_cols) - 1, row = max_rows; row > 0; row--)
			for (int col = 0; col < max_cols; col++, i--)
				if ((*inDataSourceTable)[i] != 0) return row;

		return 0; // Empty table
	}

	/**
	 * Append or reuse an entire row of bytes in the specificed table.
	 *
	 * This version is suitable for tables such as instruments or commands, where
	 * the rows are completely independent.
	 *
	 * If the row data is new, the data is appended and the position of this row is
	 * returned. If it already exists, that row position is just returned.
	 */
	unsigned char Interface::AppendToTable(int inTableType, const std::vector<unsigned char>& inBytes)
	{
		Table table = GetTable(inTableType);
		TableData table_data = GetTableData(inTableType);

		bool different;
		for (int row = 0; row < table.m_RowCount; row++)
		{
			different = false;
			for (int col = 0; col < table.m_ColumnCount; col++)
			{
				if (table.m_Data[(row * table.m_ColumnCount) + col] != inBytes[col])
				{
					different = true;
					break;
				}
			}
			if (!different) return row; // Identical row data found
		}

		if (table.m_RowCount > table_data.m_TableDefinition.m_RowCount - 1)
		{
			GetCout() << "\nSong is too complex; exceeded the " << std::dec << table_data.m_TableDefinition.m_RowCount << " rows available in the \"" << table_data.m_TableDefinition.m_Name << "\" table." << std::endl;
			return 0xff;
		}

		// The row data is new
		return EditTableRow(inTableType, table.m_RowCount, inBytes) ? table.m_RowCount : 0xff; // Unsupported
	}

	/**
	 * Append or reuse a cluster of rows with bytes in the specified table.
	 *
	 * This version is suitable for tables such as wave or arpeggio, where several
	 * rows (with a loop/end marker) are needed to define the entire effect.
	 *
	 * If the cluster is new, the data is appended and the position of the first row
	 * of it is returned. If it already exists, its first row is just returned.
	 *
	 * NOTE: Use relative wrap markers. They will be changed to absolute if needed
	 * when using PushAllDataToMemory() in the end.
	 *
	 * For single column tables such as arpeggio, use {{ x }} for byte values.
	 */
	unsigned char Interface::AppendClusterToTable(int inTableType, const std::vector<std::vector<unsigned char>>& inBytesCluster)
	{
		Table table = GetTable(inTableType);
		TableData table_data = GetTableData(inTableType);

		bool different;
		for (int row = 0; row < table.m_RowCount * table.m_ColumnCount; row += table.m_ColumnCount) // Loop all rows with data in the table
		{
			for (auto it_cluster = inBytesCluster.begin(); it_cluster != inBytesCluster.end(); ++it_cluster) // Loop the rows in the cluster
			{
				different = false;
				for (auto it_byte = (*it_cluster).begin(); it_byte != (*it_cluster).end(); ++it_byte) // Loop the bytes in the row
				{
					if (table.m_Data[row + ((it_cluster - inBytesCluster.begin()) * table.m_ColumnCount + (it_byte - (*it_cluster).begin()))] != *it_byte)
					{
						different = true;
						break;
					}
				}
				if (different) break;
			}
			if (!different) return row / table.m_ColumnCount;
		}

		// The cluster of rows is new
		for (auto it = inBytesCluster.begin(); it != inBytesCluster.end(); ++it)
			if (!EditTableRow(inTableType, table.m_RowCount + (it - inBytesCluster.begin()), *it))
				return 0xff; // Unsupported

		return table.m_RowCount;
	}

	/**
	 * Modify an entire row of bytes in the specified table.
	 *
	 * NOTE: You should always try to use AppendToTable() instead as it automatically
	 * ensures that row data is reused whenever possible.
	 */
	bool Interface::EditTableRow(int inTableType, int inRow, std::vector<unsigned char> inBytes)
	{
		if(!IsTableSupported(inTableType, true))
			return false;

		if (inTableType == TableType::Commands)
		{
			bool command_supported = IsCommandSupported(inBytes[m_CommandFormat.m_BytePosition], true); // Warning only
			if (!command_supported) return false;
		}

		TableData table_data = GetTableData(inTableType);
		int column_count = table_data.m_TableDefinition.m_ColumnCount;

		// The number of vector bytes must fit a row in this table
		assert(inBytes.size() == column_count);

		int data_index = inRow * column_count;
		for (unsigned char byte : inBytes)
			(*table_data.m_DataSourceTable)[data_index++] = byte;

		PushTableToSource(table_data);
		return true;
	}

	/**
	 * Change the description for a row in a table that supports this.
	 */
	bool Interface::EditTableRowText(int inTableType, int inRow, const std::string& inText)
	{
		if(!IsTableSupported(inTableType, true))
			return false;

		TableData table_data = GetTableData(inTableType);
		if (table_data.m_TableDefinition.m_TextFieldSize == 0)
		{
			GetCout() << "WARNING: The \"" << table_data.m_TableDefinition.m_Name << "\" table in this driver does not have descriptions." << std::endl;
			return false;
		}

		std::shared_ptr<DataSourceTableText> table_text = std::make_shared<DataSourceTableText>(
			table_data.m_TableDefinition.m_ID,
			table_data.m_TableDefinition.m_RowCount,
			m_DriverInfo->GetAuxilaryDataCollection().GetTableText()
		);

		// SF2 actually allows the description to be much longer than what can be seen in the editor
		(*table_text)[inRow] = inText;
		table_text->PushDataToSource();

		return true;
	}

	/**
	 * Read a row of bytes in a table.
	 */
	std::vector<unsigned char> Interface::ReadTableRow(int inTableType, int inRow)
	{
		if (!IsTableSupported(inTableType, true))
			return std::vector<unsigned char>();

		TableData table_data = GetTableData(inTableType);
		int column_count = table_data.m_TableDefinition.m_ColumnCount;

		std::vector<unsigned char> bytes;
		int data_index = inRow * column_count;
		for (int i = 0; i < column_count; i++)
			bytes.push_back((*table_data.m_DataSourceTable)[data_index++]);

		return bytes;
	}

	/**
	 * Set the SID model to 6581 or 8580.
	 * 
	 * @origin	runtime/editor/screens/screen_edit.cpp
	 */
	void Interface::SetSIDModel(int inSidModel)
	{
		auto& hardware_preferences = m_DriverInfo->GetAuxilaryDataCollection().GetHardwarePreferences();
		hardware_preferences.SetSIDModel(inSidModel == 6581
			? AuxilaryDataHardwarePreferences::SIDModel::MOS6581
			: AuxilaryDataHardwarePreferences::SIDModel::MOS8580);
	}

	/**
	 * Get the count of used rows in the specified table.
	 */
	const int Interface::GetCount(int inTableType)
	{
		if (!IsTableSupported(inTableType, true))
			return -1;

		return GetLastUnusedRow(GetTableData(inTableType).m_DataSourceTable);
	}

	/**
	 * Return the block in the C64 memory with the specified table.
	 */
	Interface::Block Interface::GetRangeTable(int inTableType)
	{
		if (!IsTableSupported(inTableType, true))
			return Block();

		TableData table_data = GetTableData(inTableType);

		std::stringstream description;
		description << "Table \"" << table_data.m_TableDefinition.m_Name << "\" in C64 memory";

		return { description.str(), table_data.m_TableDefinition.m_Address, GetAllMemory(), (unsigned int)table_data.m_DataSourceTable->GetSize() };
	}

}
