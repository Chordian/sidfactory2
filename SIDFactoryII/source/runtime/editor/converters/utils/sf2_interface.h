#pragma once

#include "runtime/editor/datasources/datasource_table.h"
#include "runtime/editor/driver/driver_info.h"
#include "runtime/editor/driver/driver_state.h"
#include "runtime/editor/converters/utils/consoleostreambuffer.h"

#include <memory>
#include <vector>
#include <map>
#include <ostream>

#define TABLE_INSTR		SF2::Interface::TableType::Instruments
#define TABLE_CMDS		SF2::Interface::TableType::Commands
#define TABLE_WAVE		SF2::Interface::TableType::Wave
#define TABLE_PULSE		SF2::Interface::TableType::Pulse
#define TABLE_FILTER	SF2::Interface::TableType::Filter
#define TABLE_HR		SF2::Interface::TableType::HR
#define TABLE_ARP		SF2::Interface::TableType::Arpeggio
#define TABLE_TEMPO		SF2::Interface::TableType::Tempo
#define TABLE_INIT		SF2::Interface::TableType::Init

namespace Foundation
{
	class IPlatform;
	class Viewport;
	class TextField;
	class Keyboard;
	class Mouse;
	class AudioStream;
}

namespace Emulation
{
	class CPUMemory;
}

namespace Editor
{
	class DataSourceOrderList;
	class DataSourceSequence;
	class ComponentConsole;
}

namespace Converter
{
	class Misc;
}

namespace SF2
{
	class Interface
	{
	private:
		class COutStream : public std::ostream
		{
		public:
			COutStream(std::streambuf* inBuffer)
				: std::ostream(inBuffer)
			{
			}
		};

	public:

		enum TableType : int
		{
			Instruments,
			Commands,
			Wave,
			Pulse,
			Filter,
			HR,
			Arpeggio,
			Tempo,
			Init,
			_MAX,
		};

		enum Command : unsigned char
		{
			Cmd_Slide			= 0x00,
			Cmd_Vibrato			= 0x01,
			Cmd_Portamento		= 0x02,
			Cmd_Arpeggio		= 0x03,
			Cmd_Fret			= 0x04,
			Cmd_ADSR_Note		= 0x08,
			Cmd_ADSR_Persist	= 0x09,
			Cmd_Index_Filter	= 0x0a,
			Cmd_Index_Wave		= 0x0b,
			Cmd_Index_Pulse		= 0x0c, // Added in driver 11.02
			Cmd_Tempo			= 0x0d,	// Added in driver 11.02
			Cmd_Volume			= 0x0e, // Added in driver 11.02
			Cmd_Demo_Flag		= 0x0f
		};

		struct WrapFormat
		{
			int m_ByteIDPosition;						// If -1 the table doesn't use a wrap marker
			unsigned char m_ByteIDMask;
			std::vector<unsigned char> m_ByteID;
			int m_ByteWrapPosition;
			unsigned char m_ByteWrapMask;
		};

		struct DriverDetails
		{
			/* From driver_info.cpp */

			std::string m_DriverName;
			unsigned char m_DriverVersionMajor;
			unsigned char m_DriverVersionMinor;
			int m_TrackCount;
			int m_SequenceCount;

			/* Converter only */

			int m_TableColCount[TableType::_MAX];		// If 0 the table is not defined for this driver
			int m_TableMaxRows[TableType::_MAX];
			WrapFormat m_TableWrap[TableType::_MAX];
		};

		struct CommandFormat
		{
			int m_BytePosition;
			unsigned char m_ByteMask;
		};

		struct Block
		{
			std::string sh_Description;
			unsigned int sh_Address;
			unsigned char* sh_Buffer;
			unsigned int sh_Count;
		};

		struct Range
		{
			unsigned short m_StartAddress;
			unsigned short m_EndAddress;
		};

		struct Table
		{
			std::string m_Name = "";
			int m_ColumnCount;
			int m_RowCount;
			std::vector<unsigned char> m_Data;
		};

		struct TableData
		{
			std::shared_ptr<Editor::DataSourceTable> m_DataSourceTable;
			Editor::DriverInfo::TableDefinition m_TableDefinition;
		};

		Interface(Foundation::IPlatform* platform, Editor::ComponentConsole& inConsole);
		~Interface();

		std::ostream& GetCout();

		bool LoadFile(const std::string& inFilename);
		std::shared_ptr<Utility::C64File> GetResult();

		unsigned char* GetAllMemory() const;
		Range GetRangeSF2();

		std::vector<unsigned char> GetContainerOrderList(int inTrack);
		std::vector<unsigned char> GetContainerSequence(int inSequenceIndex);
		Table GetTable(int inTableType);
		TableData GetTableData(int inTableType);
		WrapFormat GetWrapFormat(int inTableType);

		bool AppendToOrderList(int inTrack, std::vector<unsigned char> inBytes);
		bool AppendToSequence(int inSequenceIndex, std::vector<unsigned char> inBytes);
		unsigned char AppendToTable(int inTableType, const std::vector<unsigned char>& inBytes);
		unsigned char AppendToTable(int inTableType, const std::vector<std::vector<unsigned char>>& inBytesCluster);
		bool EditTableRow(int inTableType, int inRow, std::vector<unsigned char> inBytes);
		bool EditTableRowText(int inTableType, int inRow, const std::string& inText);
		std::vector<unsigned char> ReadTableRow(int inTableType, int inRow);
		void SetSIDModel(int inSidModel);
		const int GetCount(int inTableType);
		std::string GetDriverName();
		std::string GetCommandName(unsigned char inCommand);

		bool PushAllDataToMemory(bool inFixRelativeMarkers);
		void PushTableToSource(TableData inDataSourceTable);

		Block GetRangeOrderList(int inTrack);
		Block GetRangeSequence(unsigned int inSequenceIndex);
		Block GetRangeTable(int inTableType);

	private:

		void InitData();
		void ParseDriverDetails();
		bool IsTableSupported(int inTableType, bool inCoutOnError = false);
		bool IsCommandSupported(unsigned char inCommand, bool inCoutOnWarning = false);

		const int GetLastUnusedRow(std::shared_ptr<Editor::DataSourceTable> inDataSourceTable) const;

		Foundation::IPlatform* m_Platform;
		Emulation::CPUMemory* m_CPUMemory;

		std::shared_ptr<Editor::DriverInfo> m_DriverInfo;
		Editor::DriverState m_DriverState;

		std::vector<std::shared_ptr<Editor::DataSourceOrderList>> m_OrderListDataSources;
		std::vector<std::shared_ptr<Editor::DataSourceSequence>> m_SequenceDataSources;

		std::vector<unsigned char> m_SupportedCommands;
		std::map<unsigned char, std::string> m_CommandName;
		DriverDetails m_DriverDetails;
		CommandFormat m_CommandFormat;
		unsigned char* m_EntireBlock;
		Range m_Range;
		std::vector<unsigned char> m_CommandChecked;

		Editor::ConsoleOStreamBuffer m_StreamOutputBuffer;
		std::shared_ptr<COutStream> m_COutStream;
	};
}