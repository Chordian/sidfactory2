#pragma once

#include <vector>
#include "runtime/editor/driver/driver_info.h"

namespace Emulation
{
	class IMemoryRandomReadAccess;
	class CPUMemory;
}

namespace Utility
{
	class C64FileWriter;
}

namespace Editor
{
	class DataSourceTable;

	namespace DriverUtils
	{
		std::shared_ptr<DataSourceTable> CreateTableDataSource(const DriverInfo::TableDefinition& inTableDefinition, Emulation::CPUMemory* inCPUMemory);
		unsigned char GetHighestSequenceIndexUsed(const Editor::DriverInfo& inDriverInfo, const Emulation::IMemoryRandomReadAccess& inMemoryReader);
		std::vector<int> GetSequenceUsageCount(const Editor::DriverInfo& inDriverInfo, const Emulation::IMemoryRandomReadAccess& inMemoryReader);
		unsigned char GetFirstUnusedSequenceIndex(const Editor::DriverInfo& inDriverInfo, const Emulation::IMemoryRandomReadAccess& inMemoryReader);
		unsigned char GetHighestInstrumentIndexUsed(const Editor::DriverInfo& inDriverInfo, const Emulation::IMemoryRandomReadAccess& inMemoryReader);
		unsigned char GetHighestCommandIndexUsed(const Editor::DriverInfo& inDriverInfo, const Emulation::IMemoryRandomReadAccess& inMemoryReader);
		unsigned char GetHighestTableRowUsedIndex(const Editor::DriverInfo::TableDefinition& inTableDefinition, const Emulation::IMemoryRandomReadAccess& inMemoryReader);
		std::vector<unsigned short> GetOrderListsLength(const Editor::DriverInfo& inDriverInfo, const Emulation::IMemoryRandomReadAccess& inMemoryReader);
		unsigned short GetSequenceLength(unsigned short inSequenceIndex, const Editor::DriverInfo& inDriverInfo, const Emulation::IMemoryRandomReadAccess& inMemoryReader);
		unsigned short GetEndOfMusicDataAddress(const Editor::DriverInfo& inDriverInfo, const Emulation::IMemoryRandomReadAccess& inMemoryReader);
		unsigned short GetEndOfFileAddress(const Editor::DriverInfo& inDriverInfo, const Emulation::IMemoryRandomReadAccess& inMemoryReader);

		void InsertIRQ(const Editor::DriverInfo& inDriverInfo, Utility::C64FileWriter& inFileWriter);
	}
}