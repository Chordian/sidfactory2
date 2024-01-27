#include "packer.h"
#include "packing_utils.h"
#include "runtime/editor/driver/driver_info.h"
#include "runtime/editor/driver/driver_utils.h"
#include "runtime/editor/auxilarydata/auxilary_data_collection.h"
#include "runtime/editor/auxilarydata/auxilary_data_songs.h"
#include "runtime/emulation/cpumos6510.h"
#include "utils/c64file.h"

#include <algorithm>

//#define _DEBUG_PACKER
#ifdef _DEBUG_PACKER
#include <windows.h>
#endif //_DEBUG_PACKER

namespace Editor
{
	void CopyData(const Emulation::CPUMemory& inSource, Utility::C64File& inDestination, unsigned short inSourceAddres, unsigned short inDestinationAddress, unsigned short inLength)
	{
		for (unsigned short i = 0; i < inLength; ++i)
			inDestination[inDestinationAddress + i] = inSource[inSourceAddres + i];
	}


	Packer::Packer(Emulation::CPUMemory& inCPUMemory, const DriverInfo& inDriverInfo, unsigned short inDestinationAddress, unsigned char inLowestZP)
		: m_CPUMemory(inCPUMemory)
		, m_DriverInfo(inDriverInfo)
		, m_DestinationAddress(inDestinationAddress)
		, m_LowestZP(inLowestZP)
		, m_HighestUsedSequenceIndex(0)
		, m_OrderListPointersDataSectionLowID(0)
		, m_OrderListPointersDataSectionHighID(0)
		, m_SequencePointersDataSectionLowID(0)
		, m_SequencePointersDataSectionHighID(0)
	{
		FOUNDATION_ASSERT(inDriverInfo.IsValid());

		// Compute the destination address delta. The driver and data are processed for the destination address at address 0x1000 in the data container, but the load 
		// address of the file generated is altered to the destination address before saving it to disk.

		ZeroPageRange zp_range = GetZeroPageRangeFromDriver(inCPUMemory, inDriverInfo);

		FOUNDATION_ASSERT(zp_range.m_LowestZeroPage > 0);
		FOUNDATION_ASSERT(zp_range.m_LowestZeroPage <= zp_range.m_HighestZeroPage);

		m_CurrentLowestZP = zp_range.m_LowestZeroPage;

		m_DestinationAddressDelta = m_DestinationAddress - m_DriverInfo.GetDescriptor().m_DriverCodeTop;

		m_CPUMemory.Lock();

		FetchTables();
		FetchOrderListPointers();
		FetchSequencePointers();
		FetchOrderLists();
		FetchSequences();

		// Sort data descriptors
		std::sort(m_DataSectionList.begin(), m_DataSectionList.end(), [](const auto& inA, const auto& inB)
		{ 
			return (inA.m_SourceAddress < inB.m_SourceAddress); 
		});

		const bool requires_multi_song_patch = m_DriverInfo.GetAuxilaryDataCollection().GetSongs().GetSongCount() > 1;

		unsigned short end_address = ComputeDestinationAddresses();

		if (requires_multi_song_patch)
			end_address += GetMultiSongPatchSize();

		m_OutputData = CreateOutputDataContainer(m_DriverInfo.GetDescriptor().m_DriverCodeTop, end_address);

		unsigned short data_address = CopyDataToOutputContainer();

		AdjustOrderListPointers();
		AdjustSequencePointers();

		ProcessDriverCode();

		if (requires_multi_song_patch)
			ApplyMultiSongPatch(data_address);

		// Move the top address of the output data container, to the selected destination address (the code and data have been generated specifically for the destination address already)
		m_OutputData->MoveDataToTopAddress(m_DestinationAddress);

		m_CPUMemory.Unlock();
	}


	Packer::~Packer()
	{

	}


	std::shared_ptr<Utility::C64File> Packer::GetResult() const
	{
		return m_OutputData;
	}


	const unsigned int Packer::AddDataSection(unsigned short inAddress, unsigned short inSize)
	{
		unsigned int section_id = static_cast<unsigned int>(m_DataSectionList.size());
		m_DataSectionList.push_back({ section_id, inAddress, inSize });

		return section_id;
	}


	const Packer::DataSection* Packer::GetDataSection(int inID) const
	{
		for (const DataSection& data_section : m_DataSectionList)
		{
			if (data_section.m_ID == inID)
				return &data_section;
		}

		return nullptr;
	}



	void Packer::FetchTables()
	{
		const std::vector<DriverInfo::TableDefinition>& table_definitions = m_DriverInfo.GetTableDefinitions();

		for (const auto& table : table_definitions)
		{
			if (table.m_DataLayout == DriverInfo::TableDefinition::DataLayout::ColumnMajor)
			{
				const unsigned short data_size = [&]()
				{
					if (table.m_Type == DriverInfo::TableType::Instruments)
						return static_cast<unsigned short>(DriverUtils::GetHighestInstrumentIndexUsed(m_DriverInfo, m_CPUMemory)) + 1;
					if (table.m_Type == DriverInfo::TableType::Commands)
						return static_cast<unsigned short>(DriverUtils::GetHighestCommandIndexUsed(m_DriverInfo, m_CPUMemory)) + 1;

					return static_cast<unsigned short>(DriverUtils::GetHighestTableRowUsedIndex(table, m_CPUMemory)) + 1;
				}();

				for (unsigned short i = 0; i < table.m_ColumnCount; ++i)
				{
					const unsigned short source_address = table.m_Address + static_cast<unsigned short>(table.m_RowCount) * i;
					AddDataSection(source_address, data_size);
				}
			}
			else if (table.m_DataLayout == DriverInfo::TableDefinition::DataLayout::RowMajor)
			{
				const unsigned short data_size = [&]()
				{
					if (table.m_Type == DriverInfo::TableType::Instruments)
						return static_cast<unsigned short>(table.m_ColumnCount * (DriverUtils::GetHighestInstrumentIndexUsed(m_DriverInfo, m_CPUMemory) + 1));
					if (table.m_Type == DriverInfo::TableType::Commands)
						return static_cast<unsigned short>(table.m_ColumnCount * (DriverUtils::GetHighestCommandIndexUsed(m_DriverInfo, m_CPUMemory) + 1));

					return static_cast<unsigned short>(table.m_ColumnCount * (DriverUtils::GetHighestTableRowUsedIndex(table, m_CPUMemory) + 1));
				}();

				const unsigned short source_address = table.m_Address;
				AddDataSection(source_address, data_size);
			}
		}
	}


	void Packer::FetchOrderListPointers()
	{
		const unsigned short data_size = static_cast<unsigned short>(m_DriverInfo.GetMusicData().m_TrackCount);

		const unsigned short orderlist_pointers_low_address = m_DriverInfo.GetMusicData().m_TrackOrderListPointersLowAddress;
		m_OrderListPointersDataSectionLowID = AddDataSection(orderlist_pointers_low_address, data_size);
		const unsigned short orderlist_pointers_high_address = m_DriverInfo.GetMusicData().m_TrackOrderListPointersHighAddress;
		m_OrderListPointersDataSectionHighID = AddDataSection(orderlist_pointers_high_address, data_size);
	}


	void Packer::FetchSequencePointers()
	{
		unsigned char higest_sequence_used = DriverUtils::GetHighestSequenceIndexUsed(m_DriverInfo, m_CPUMemory);
		unsigned short data_size = static_cast<unsigned short>(higest_sequence_used) + 1;

		const unsigned short sequence_pointers_low_address = m_DriverInfo.GetMusicData().m_SequencePointersLowAddress;
		m_SequencePointersDataSectionLowID = AddDataSection(sequence_pointers_low_address, data_size);
		const unsigned short sequence_pointers_high_address = m_DriverInfo.GetMusicData().m_SequencePointersHighAddress;
		m_SequencePointersDataSectionHighID = AddDataSection(sequence_pointers_high_address, data_size);
	}


	void Packer::FetchOrderLists()
	{
		std::vector<unsigned short> orderlist_length_list = DriverUtils::GetOrderListsLength(m_DriverInfo, m_CPUMemory);
		const auto& music_data = m_DriverInfo.GetMusicData();
		const unsigned char song_count = m_DriverInfo.GetAuxilaryDataCollection().GetSongs().GetSongCount();

		for (int i = 0; i < music_data.m_TrackCount * song_count; ++i)
		{
			const unsigned short order_list_address = music_data.m_OrderListTrack1Address + music_data.m_OrderListSize * i;
			m_OrderListDataSectionIDList.push_back(AddDataSection(order_list_address, orderlist_length_list[i]));
		}
	}


	void Packer::FetchSequences()
	{
		m_HighestUsedSequenceIndex = DriverUtils::GetHighestSequenceIndexUsed(m_DriverInfo, m_CPUMemory);
		const auto& music_data = m_DriverInfo.GetMusicData();

		for (int i = 0; i <= m_HighestUsedSequenceIndex; ++i)
		{
			const unsigned short sequence_address = music_data.m_Sequence00Address + music_data.m_SequenceSize * i;
			const unsigned short sequence_length = DriverUtils::GetSequenceLength(i, m_DriverInfo, m_CPUMemory);

			m_SequenceDataSectionIDList.push_back(AddDataSection(sequence_address, sequence_length));
		}
	}


	unsigned short Packer::ComputeDestinationAddresses()
	{
		FOUNDATION_ASSERT(m_DataSectionList.size() > 0);

		unsigned short address = m_DataSectionList[0].m_SourceAddress;

		for (DataSection& data_section : m_DataSectionList)
		{
#ifdef _DEBUG_PACKER
			static char debug_buffer[65536];
			sprintf_s<65536>(debug_buffer, "DataSection: @0x%04x - 0x%04x -> 0x%04x\n", data_section.m_SourceAddress, data_section.m_SourceSize, address);
			OutputDebugString(debug_buffer);
#endif //_DEBUG_PACKER

			data_section.m_DestinationAddress = address;
			address += data_section.m_SourceSize;
		}

		return address;
	}


	std::shared_ptr<Utility::C64File> Packer::CreateOutputDataContainer(unsigned short inTopAddress, unsigned short inEndAddress)
	{
		return Utility::C64File::CreateAsContainer(inTopAddress, inEndAddress);
	}


	unsigned short Packer::CopyDataToOutputContainer()
	{
		FOUNDATION_ASSERT(m_OutputData != nullptr);
		
		const unsigned short driver_top_address = m_DriverInfo.GetDescriptor().m_DriverCodeTop;
		const unsigned short driver_size = m_DriverInfo.GetDescriptor().m_DriverSize;

		CopyData(m_CPUMemory, *m_OutputData, driver_top_address, driver_top_address, driver_size);

		unsigned short data_address = driver_top_address + driver_size;

		for (const DataSection& data_section : m_DataSectionList)
		{
			FOUNDATION_ASSERT(data_address == data_section.m_DestinationAddress);
			CopyData(m_CPUMemory, *m_OutputData, data_section.m_SourceAddress, data_section.m_DestinationAddress, data_section.m_SourceSize);

			data_address += data_section.m_SourceSize;
		}

		return data_address;
	}


	void Packer::AdjustOrderListPointers()
	{
		const unsigned short order_list_pointers_low_address = GetDataSection(m_OrderListPointersDataSectionLowID)->m_DestinationAddress;
		const unsigned short order_list_pointers_high_address = GetDataSection(m_OrderListPointersDataSectionHighID)->m_DestinationAddress;

		const auto& music_data = m_DriverInfo.GetMusicData();

		int offset = 0;

		for (int order_list_id : m_OrderListDataSectionIDList)
		{
			unsigned short order_list_address = GetDataSection(order_list_id)->m_DestinationAddress + m_DestinationAddressDelta;

			m_OrderListAdressList.push_back(order_list_address);

			if (offset < static_cast<int>(music_data.m_TrackCount))
			{
				(*m_OutputData)[order_list_pointers_low_address + offset] = static_cast<unsigned char>(order_list_address & 0xff);
				(*m_OutputData)[order_list_pointers_high_address + offset] = static_cast<unsigned char>((order_list_address >> 8) & 0xff);

				++offset;
			}
		}
	}


	void Packer::AdjustSequencePointers()
	{
		const unsigned short sequence_pointers_low_address = GetDataSection(m_SequencePointersDataSectionLowID)->m_DestinationAddress;
		const unsigned short sequence_pointers_high_address = GetDataSection(m_SequencePointersDataSectionHighID)->m_DestinationAddress;

		int offset = 0;

		for (int sequence_id : m_SequenceDataSectionIDList)
		{
			unsigned short sequence_address = GetDataSection(sequence_id)->m_DestinationAddress + m_DestinationAddressDelta;

			(*m_OutputData)[sequence_pointers_low_address + offset] = static_cast<unsigned char>(sequence_address & 0xff);
			(*m_OutputData)[sequence_pointers_high_address + offset] = static_cast<unsigned char>((sequence_address >> 8) & 0xff);

			++offset;
		}
	}


	static const unsigned char multi_song_patch_code[] =
	{
		0x85, 0xfe,			// c000   STA $FE		// ZP lowest
		0x0a,				// c002   ASL A
		0x18,				// c003   CLC
		0x65, 0xfe,			// c004   ADC $FE		// ZP lowest
		0xaa,				// c006   TAX
		0xa0, 0x00,			// c007   LDY #$00
		0xbd, 0x00, 0xc1,	// c009   LDA $C100,X	// Order list pointer source low
		0x99, 0x00, 0xc2,	// c00c   STA $C200,Y	// Driver order list  pointer low 
		0xbd, 0x10, 0xc1,	// c00f   LDA $C110,X	// Order list pointer source high
		0x99, 0x03, 0xc2,	// c012   STA $C203,Y	// Driver order list  pointer high
		0xe8,				// c015   INX
		0xc8,				// c016   INY
		0xc0, 0x03,			// c017   CPY #$03
		0xd0, 0xee,			// c019   BNE $C009
		0xa5, 0xfe,			// c01b   LDA $FE		// ZP lowest
		0x4c, 0x03, 0x10	// c01d   JMP $1000		// Jump to vector originally jumped to by jump at $1000
	};

	unsigned short Packer::GetMultiSongPatchSize()
	{
		const unsigned char song_count = m_DriverInfo.GetAuxilaryDataCollection().GetSongs().GetSongCount();
		const unsigned char track_count = m_DriverInfo.GetMusicData().m_TrackCount;

		const unsigned short order_list_pointer_size = static_cast<unsigned short>(song_count * track_count) * 2;
		const unsigned short patch_code_size = sizeof(multi_song_patch_code);

		return order_list_pointer_size + patch_code_size;
	}


	void Packer::ApplyMultiSongPatch(unsigned short inTargetAddress)
	{
		static unsigned short zp_fix_offsets[] = { 0x0001, 0x0005, 0x001c };

		static unsigned short order_list_pointer_low_read_fix_offset = 0x000a;
		static unsigned short order_list_pointer_high_read_fix_offset = 0x0010;
		static unsigned short order_list_pointer_low_write_fix_offset = 0x000d;
		static unsigned short order_list_pointer_high_write_fix_offset = 0x0013;

		static unsigned short jump_vector_offset = 0x001e;

		unsigned short order_list_count = static_cast<unsigned short>(m_OrderListAdressList.size());

		unsigned short offset = 0;

		for (unsigned short order_list_address : m_OrderListAdressList)
		{
			(*m_OutputData)[inTargetAddress + offset] = static_cast<unsigned char>(order_list_address & 0xff);
			(*m_OutputData)[inTargetAddress + order_list_count + offset] = static_cast<unsigned char>(order_list_address >> 8);
		
			++offset;
		}

		unsigned short code_patch_target_address = inTargetAddress + 2 * order_list_count;
		unsigned short patch_code_size = sizeof(multi_song_patch_code);

		for (unsigned short i = 0; i < patch_code_size; ++i)
			(*m_OutputData)[code_patch_target_address + i] = multi_song_patch_code[i];

		// Fix zero pages in patch code
		for (unsigned int i = 0; i < 3; ++i)
			(*m_OutputData)[code_patch_target_address + zp_fix_offsets[i]] = m_LowestZP;

		// Patch the orderlist read addresses
		const unsigned short order_list_read_low_address = inTargetAddress + m_DestinationAddressDelta;
		const unsigned short order_list_read_high_address = inTargetAddress + order_list_count + m_DestinationAddressDelta;

		(*m_OutputData)[code_patch_target_address + order_list_pointer_low_read_fix_offset] = static_cast<unsigned char>(order_list_read_low_address & 0xff);
		(*m_OutputData)[code_patch_target_address + order_list_pointer_low_read_fix_offset + 1] = static_cast<unsigned char>(order_list_read_low_address >> 8);
		(*m_OutputData)[code_patch_target_address + order_list_pointer_high_read_fix_offset] = static_cast<unsigned char>((order_list_read_high_address) & 0xff);
		(*m_OutputData)[code_patch_target_address + order_list_pointer_high_read_fix_offset + 1] = static_cast<unsigned char>((order_list_read_high_address) >> 8);

		// Patch driver order list pointer destination
		const unsigned short order_list_pointers_low_address = GetDataSection(m_OrderListPointersDataSectionLowID)->m_DestinationAddress + m_DestinationAddressDelta;
		const unsigned short order_list_pointers_high_address = GetDataSection(m_OrderListPointersDataSectionHighID)->m_DestinationAddress + m_DestinationAddressDelta;

		(*m_OutputData)[code_patch_target_address + order_list_pointer_low_write_fix_offset] = static_cast<unsigned char>(order_list_pointers_low_address & 0xff);
		(*m_OutputData)[code_patch_target_address + order_list_pointer_low_write_fix_offset + 1] = static_cast<unsigned char>(order_list_pointers_low_address >> 8);
		(*m_OutputData)[code_patch_target_address + order_list_pointer_high_write_fix_offset] = static_cast<unsigned char>((order_list_pointers_high_address) & 0xff);
		(*m_OutputData)[code_patch_target_address + order_list_pointer_high_write_fix_offset + 1] = static_cast<unsigned char>((order_list_pointers_high_address) >> 8);


		// Patch the jump vector
		const unsigned short init_address = m_DriverInfo.GetDriverCommon().m_InitAddress;
		const unsigned short init_jump_vector = m_OutputData->GetWord(init_address + 1);

		(*m_OutputData)[code_patch_target_address + jump_vector_offset] = static_cast<unsigned char>(init_jump_vector & 0xff);
		(*m_OutputData)[code_patch_target_address + jump_vector_offset + 1] = static_cast<unsigned char>(init_jump_vector >> 8);

		const unsigned short relocated_code_patch_target_address = code_patch_target_address + m_DestinationAddressDelta;

		(*m_OutputData)[init_address + 1] = static_cast<unsigned char>(relocated_code_patch_target_address & 0xff);
		(*m_OutputData)[init_address + 2] = static_cast<unsigned char>(relocated_code_patch_target_address >> 8);
	}


	unsigned short Packer::GetRelocatedVector(unsigned short inVectorAddress) const
	{
		for (const auto& data_section : m_DataSectionList)
		{
			if (inVectorAddress >= data_section.m_SourceAddress && inVectorAddress < data_section.m_SourceAddress + data_section.m_SourceSize)
			{
				unsigned short address_offset = inVectorAddress - data_section.m_SourceAddress;
				return data_section.m_DestinationAddress + address_offset;
			}
		}

		return inVectorAddress;
	}


	void Packer::ProcessDriverCode()
	{
		auto requires_relocation = [](Emulation::CPUmos6510::AddressingMode inAddressingMode)
		{
			switch (inAddressingMode)
			{
			case Emulation::CPUmos6510::am_ABS:
			case Emulation::CPUmos6510::am_ABX:
			case Emulation::CPUmos6510::am_ABY:
			case Emulation::CPUmos6510::am_IND:
				return true;
            default:
                break;
			}

			return false;
		};

		auto requires_zeropage_check = [](Emulation::CPUmos6510::AddressingMode inAddressingMode)
		{
			switch (inAddressingMode)
			{
			case Emulation::CPUmos6510::am_ZP:
			case Emulation::CPUmos6510::am_ZPX:
			case Emulation::CPUmos6510::am_ZPY:
			case Emulation::CPUmos6510::am_IZX:
			case Emulation::CPUmos6510::am_IZY:
				return true;
            default:
                break;
			}
        
			return false;
		};

		const int top_address = m_DriverInfo.GetDescriptor().m_DriverCodeTop;
		const int bottom_address = top_address + m_DriverInfo.GetDescriptor().m_DriverCodeSize;

		int address = top_address;

		while (address < bottom_address)
		{
			const unsigned char opcode = (*m_OutputData)[address];
			const unsigned char opcode_size = Emulation::CPUmos6510::GetOpcodeByteSize(opcode);
			const Emulation::CPUmos6510::AddressingMode opcode_addressing_mode = Emulation::CPUmos6510::GetOpcodeAddressingMode(opcode);

			if (requires_relocation(opcode_addressing_mode))
			{
				FOUNDATION_ASSERT(opcode_size == 3);

				const unsigned short code_vector = m_OutputData->GetWord(address + 1);
				const unsigned short relocated_code_vector = [&]() -> const unsigned short
				{
					if (code_vector >= 0xd000 && code_vector <= 0xdfff)
						return GetRelocatedVector(code_vector);

					return GetRelocatedVector(code_vector) + m_DestinationAddressDelta;
				}();

				if (code_vector != relocated_code_vector)
				{
					(*m_OutputData)[address + 1] = static_cast<unsigned char>(relocated_code_vector & 0xff);
					(*m_OutputData)[address + 2] = static_cast<unsigned char>((relocated_code_vector >> 8) & 0xff);
				}
			}

            // Relocate zp addresses
			if (requires_zeropage_check(opcode_addressing_mode))
			{
				FOUNDATION_ASSERT(opcode_size == 2);
            
				const unsigned char zero_page = m_OutputData->GetByte(address + 1);
				const unsigned char zero_page_base = zero_page - m_CurrentLowestZP;
				const unsigned char zero_page_relocated = zero_page_base + m_LowestZP;

				(*m_OutputData)[address + 1] = zero_page_relocated;
			}

			address += static_cast<unsigned short>(opcode_size);
		}
	}
}
