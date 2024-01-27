#include "runtime/editor/driver/driver_info.h"
#include "runtime/editor/driver/idriver_architecture.h"
#include "runtime/editor/driver/driver_architecture_sidfactory2.h"
#include "runtime/editor/auxilarydata/auxilary_data_collection.h"

#include "runtime/emulation/cpumemory.h"

#include "utils/c64file.h"

#include "foundation/base/assert.h"

namespace Editor
{
	DriverInfo::DriverInfo()
		: m_IsValid(false)
		, m_ParsedDescriptorBlocks(0)
		, m_FoundRequiredTableInstruments(false)
		, m_FoundRequiredTableCommands(false)
		, m_TopAddress(0x0000)
		, m_HasEditData(false)
		, m_AuxilaryDataCollection(std::make_unique<AuxilaryDataCollection>())
	{
	}


	DriverInfo::~DriverInfo()
	{

	}

	//-----------------------------------------------------------------------------------------------

	void DriverInfo::Parse(const Utility::C64File& inFile)
	{
		FOUNDATION_ASSERT(!m_IsValid);
		FOUNDATION_ASSERT(m_ParsedDescriptorBlocks == 0);

		// Search for start address
		unsigned short top_address = inFile.GetTopAddress();

		if (inFile.GetWord(top_address) == ExpectedFileIDNumber)
		{
			m_TopAddress = top_address;

			if (ParseHeader(inFile))
				m_IsValid = m_FoundRequiredTableInstruments && m_FoundRequiredTableCommands && HasParsedRequiredBlocks();
			if (ParseAuxilaryData(inFile))
				m_HasEditData = true;
		}
	}

	bool DriverInfo::IsValid() const
	{
		return m_IsValid;
	}

	bool DriverInfo::IsParticalyValid() const
	{
		if (!IsValid())
		{
			for (int i = 0; i < HeaderBlockID::_IDBlock_Count; ++i)
			{
				if (HasParsedHeaderBlock(static_cast<HeaderBlockID>(i)))
					return true;
			}
		}

		return false;
	}

	//-----------------------------------------------------------------------------------------------

	const DriverInfo::Descriptor& DriverInfo::GetDescriptor() const
	{
		FOUNDATION_ASSERT(HasParsedHeaderBlock(HeaderBlockID::ID_Descriptor));
		return m_Descriptor;
	}


	const DriverInfo::DriverCommon& DriverInfo::GetDriverCommon() const
	{
		FOUNDATION_ASSERT(HasParsedHeaderBlock(HeaderBlockID::ID_DriverCommon));
		return m_DriverCommon;
	}


	const DriverInfo::MusicData& DriverInfo::GetMusicData() const
	{
		FOUNDATION_ASSERT(HasParsedHeaderBlock(HeaderBlockID::ID_MusicData));
		return m_MusicData;
	}


	const std::vector<DriverInfo::TableDefinition>& DriverInfo::GetTableDefinitions() const
	{
		FOUNDATION_ASSERT(HasParsedHeaderBlock(HeaderBlockID::ID_DriverTables));
		return m_TableDefinitions;
	}


	const std::vector<DriverInfo::TableColorRules>& DriverInfo::GetTableColorRules() const
	{
		return m_TableColorRules;
	}


	const std::vector<DriverInfo::TableInsertDeleteRules>& DriverInfo::GetTableInsertDeleteRules() const
	{
		return m_TableInsDelRules;
	}


	const std::vector<DriverInfo::TableActionRules>& DriverInfo::GetTableActionRules() const
	{
		return m_TableActionRules;
	}


	const DriverInfo::InstrumentDataDescription& DriverInfo::GetInstrumentDataDescription() const
	{
		return m_InstrumentDataDescription;
	}


	unsigned short DriverInfo::GetTopAddress() const
	{
		return m_TopAddress;
	}


	IDriverArchitecture* const DriverInfo::GetDriverArchitecture() const
	{
		FOUNDATION_ASSERT(m_DriverArchitecture != nullptr);
		return m_DriverArchitecture.get();
	}

	//-----------------------------------------------------------------------------------------------

	bool DriverInfo::HasParsedHeaderBlock(DriverInfo::HeaderBlockID inBlockID) const
	{
		int bit = 1 << inBlockID;
		return (m_ParsedDescriptorBlocks & bit) != 0;
	}

	bool DriverInfo::HasFoundHeaderBlock(DriverInfo::HeaderBlockID inBlockID) const
	{
		int bit = 1 << inBlockID;
		return (m_FoundDescriptorBlocks & bit) != 0;
	}

	bool DriverInfo::HasParsedRequiredBlocks() const
	{
		if (!HasParsedHeaderBlock(HeaderBlockID::ID_Descriptor))
			return false;
		if (!HasParsedHeaderBlock(HeaderBlockID::ID_DriverCommon))
			return false;
		if (!HasParsedHeaderBlock(HeaderBlockID::ID_DriverTables))
			return false;
		if (!HasParsedHeaderBlock(HeaderBlockID::ID_DriverInstrumentDescriptor))
			return false;
		if (!HasParsedHeaderBlock(HeaderBlockID::ID_MusicData))
			return false;
		//if (!HasParsedHeaderBlock(HeaderBlockID::ID_TableColorRules))
		//	return false;
		//if (!HasParsedHeaderBlock(HeaderBlockID::ID_TableInsertDeleteRules))
		//	return false;
		//if (!HasParsedHeaderBlock(HeaderBlockID::ID_TableActionRules))
		//	return false;
		//if (!HasParsedHeaderBlock(HeaderBlockID::ID_DriverInstrumentDataDescriptor))
		//	return false;

		return true;
	}

	//-----------------------------------------------------------------------------------------------

	bool DriverInfo::HasEditData() const
	{
		return m_HasEditData;
	}


	AuxilaryDataCollection& DriverInfo::GetAuxilaryDataCollection()
	{
		return *m_AuxilaryDataCollection;
	}


	const AuxilaryDataCollection& DriverInfo::GetAuxilaryDataCollection() const
	{
		return *m_AuxilaryDataCollection;
	}


	//-----------------------------------------------------------------------------------------------

	const DriverInfo::MusicDataMetaDataEmulationAddresses& DriverInfo::GetMusicDataMetaDataEmulationAddresses() const
	{
		FOUNDATION_ASSERT(HasParsedHeaderBlock(HeaderBlockID::ID_MusicData));
		return m_MusicDataMetaDataEmulationAddresses;
	}

	void DriverInfo::RefreshMusicData(Emulation::CPUMemory& inCPUMemory)
	{
		inCPUMemory.Lock();

		m_MusicData.m_SequencePointersLowAddress = inCPUMemory.GetWord(m_MusicDataMetaDataEmulationAddresses.m_EmulationAddressOfSequencePointersLowAddress);
		m_MusicData.m_SequencePointersHighAddress = inCPUMemory.GetWord(m_MusicDataMetaDataEmulationAddresses.m_EmulationAddressOfSequencePointersHighAddress);
		m_MusicData.m_OrderListTrack1Address = inCPUMemory.GetWord(m_MusicDataMetaDataEmulationAddresses.m_EmulationAddressOfOrderListTrack1Address);
		m_MusicData.m_Sequence00Address = inCPUMemory.GetWord(m_MusicDataMetaDataEmulationAddresses.m_EmulationAddressOfSequence00Address);

		inCPUMemory.Unlock();
	}


	//-----------------------------------------------------------------------------------------------

	void DriverInfo::SetHasParsedHeaderBlock(DriverInfo::HeaderBlockID inBlockID)
	{
		int bit = 1 << inBlockID;
		m_ParsedDescriptorBlocks |= bit;
	}

	void DriverInfo::SetHasFoundHeaderBlock(DriverInfo::HeaderBlockID inBlockID)
	{
		int bit = 1 << inBlockID;
		m_FoundDescriptorBlocks |= bit;
	}


	//-----------------------------------------------------------------------------------------------

	bool DriverInfo::ParseHeader(const Utility::C64File& inFile)
	{
		unsigned short block_address = m_TopAddress + 2;
		
		while (true)
		{
			Utility::C64FileReader reader(inFile, block_address, inFile.GetTopAddress() + static_cast<unsigned short>(inFile.GetDataSize()));
			unsigned int block_id = static_cast<unsigned int>(reader.ReadByte());

			if (block_id == HeaderBlockID::ID_End)
				return true;

			if (HasParsedHeaderBlock(static_cast<HeaderBlockID>(block_id)))
				return false;

			unsigned short block_size = static_cast<unsigned short>(reader.ReadByte());
			unsigned short expected_end_of_block_address = reader.GetReadAddress() + block_size;

			Utility::C64FileReader block_reader(inFile, reader.GetReadAddress(), expected_end_of_block_address);

			switch (block_id)
			{
			case HeaderBlockID::ID_Descriptor:
				ParseDescriptor(block_reader);
				break;
			case HeaderBlockID::ID_DriverCommon:
				ParseDriverCommon(block_reader);
				break;
			case HeaderBlockID::ID_DriverTables:
				ParseDriverTables(block_reader);
				break;
			case HeaderBlockID::ID_DriverInstrumentDescriptor:
				ParseDriverInstrumentDescriptor(block_reader);
				break;
			case HeaderBlockID::ID_MusicData:
				ParseMusicData(block_reader);
				break;
			case HeaderBlockID::ID_TableColorRules:
				ParseTableColorRules(block_reader);
				break;
			case HeaderBlockID::ID_TableInsertDeleteRules:
				ParseTableInsDelRules(block_reader);
				break;
			case HeaderBlockID::ID_TableActionRules:
				ParseTableActionRules(block_reader);
				break;
			case HeaderBlockID::ID_DriverInstrumentDataDescriptor:
				ParseDriverInstrumentDataDescriptor(block_reader);
				break;
			}

			SetHasFoundHeaderBlock(static_cast<HeaderBlockID>(block_id));

			if (block_reader.IsAtEndAddress())
				SetHasParsedHeaderBlock(static_cast<HeaderBlockID>(block_id));

			block_address = expected_end_of_block_address;
		}

		return false;
	}


	void DriverInfo::ParseDescriptor(Utility::C64FileReader& inReader)
	{
		m_Descriptor.m_DriverType = inReader.ReadByte();	
		m_Descriptor.m_DriverSize = inReader.ReadWord();
		m_Descriptor.m_DriverName = inReader.ReadNullTerminatedString();
		m_Descriptor.m_DriverCodeTop = inReader.ReadWord();
		m_Descriptor.m_DriverCodeSize = inReader.ReadWord();
		m_Descriptor.m_DriverVersionMajor = inReader.ReadByte();
		m_Descriptor.m_DriverVersionMinor = inReader.ReadByte();

		if (!inReader.IsAtEndAddress())
			m_Descriptor.m_DriverVersionRevision = inReader.ReadByte();
		else
			m_Descriptor.m_DriverVersionRevision = 0;

		m_DriverArchitecture = [driver_type = m_Descriptor.m_DriverType]()
		{
			if (driver_type == DriverArchitectureSidFactory2::GetDescriptorType())
				return std::unique_ptr<IDriverArchitecture>(new DriverArchitectureSidFactory2());

			return std::unique_ptr<IDriverArchitecture>();
		}();
	}


	void DriverInfo::ParseDriverCommon(Utility::C64FileReader& inReader)
	{
		m_DriverCommon.m_InitAddress = inReader.ReadWord();
		m_DriverCommon.m_StopAddress = inReader.ReadWord();
		m_DriverCommon.m_UpdateAddress = inReader.ReadWord();
		m_DriverCommon.m_SIDChannelOffsetAddress = inReader.ReadWord();
		m_DriverCommon.m_DriverStateAddress = inReader.ReadWord();
		m_DriverCommon.m_TickCounterAddress = inReader.ReadWord();
		m_DriverCommon.m_OrderListIndexAddress = inReader.ReadWord();
		m_DriverCommon.m_SequenceIndexAddress = inReader.ReadWord();
		m_DriverCommon.m_SequenceInUseAddress = inReader.ReadWord();
		m_DriverCommon.m_CurrentSequenceAddress = inReader.ReadWord();
		m_DriverCommon.m_CurrentTransposeAddress = inReader.ReadWord();
		m_DriverCommon.m_CurrentSequenceEventDurationAddress = inReader.ReadWord();
		m_DriverCommon.m_NextInstrumentAddress = inReader.ReadWord();
		m_DriverCommon.m_NextCommandAddress = inReader.ReadWord();
		m_DriverCommon.m_NextNoteAddress = inReader.ReadWord();
		m_DriverCommon.m_NextNoteIsTiedAddress = inReader.ReadWord();
		m_DriverCommon.m_TempoCounterAddress = inReader.ReadWord();
		m_DriverCommon.m_TriggerSyncAddress = inReader.ReadWord();
		m_DriverCommon.m_NoteEventTriggerSyncValue = inReader.ReadByte();
		m_DriverCommon.m_ReservedByte = inReader.ReadByte();
		m_DriverCommon.m_ReservedWord = inReader.ReadWord();
	}


	void DriverInfo::ParseDriverTables(Utility::C64FileReader& inReader)
	{
		while (!inReader.HasPassedEnd())
		{
			unsigned char table_type = inReader.ReadByte();

			if (table_type == 0xff)
				break;

			TableDefinition table_definition;

			table_definition.m_Type = table_type;

			table_definition.m_ID = inReader.ReadByte();
			table_definition.m_TextFieldSize = inReader.ReadByte();
			table_definition.m_Name = inReader.ReadNullTerminatedString();
			table_definition.m_DataLayout = static_cast<TableDefinition::DataLayout>(inReader.ReadByte());

			const unsigned char properties = inReader.ReadByte();
			table_definition.m_PropertyEnabledInsertDelete = (properties & TableDefinition::Properties::EnableInsertDelete) != 0;
			table_definition.m_PropertyLayoutVertically = (properties & TableDefinition::Properties::LayoutAddVertically) != 0;
			table_definition.m_PropertyIndexAsContinuousMemory = (properties & TableDefinition::Properties::IndexAsContinuousMemory) != 0;

			table_definition.m_InsertDeleteRuleID = inReader.ReadByte();
			table_definition.m_EnterActionRuleID = inReader.ReadByte();
			table_definition.m_ColorRuleID = inReader.ReadByte();

			table_definition.m_Address = inReader.ReadWord();
			table_definition.m_ColumnCount = inReader.ReadWord();
			table_definition.m_RowCount = inReader.ReadWord();

			table_definition.m_VisibleRowCount = inReader.ReadByte();

			m_TableDefinitions.push_back(table_definition);

			if (table_definition.m_Type == TableType::Instruments)
				m_FoundRequiredTableInstruments = true;
			if (table_definition.m_Type == TableType::Commands)
				m_FoundRequiredTableCommands = true;
		}
	}


	void DriverInfo::ParseDriverInstrumentDescriptor(Utility::C64FileReader& inReader)
	{
		unsigned short descriptor_count = inReader.ReadByte();

		for (unsigned short i = 0; i < descriptor_count; ++i)
		{
			std::string descriptor = inReader.ReadNullTerminatedString();
			m_InstrumentDescriptor.m_CellDescription.push_back(descriptor);
		}
	}


	void DriverInfo::ParseMusicData(Utility::C64FileReader& inReader)
	{
		m_MusicData.m_TrackCount = inReader.ReadByte();
		m_MusicData.m_TrackOrderListPointersLowAddress = inReader.ReadWord();
		m_MusicData.m_TrackOrderListPointersHighAddress = inReader.ReadWord();

		m_MusicData.m_SequenceCount = inReader.ReadByte();

		m_MusicDataMetaDataEmulationAddresses.m_EmulationAddressOfSequencePointersLowAddress = inReader.GetReadAddress();
		m_MusicData.m_SequencePointersLowAddress = inReader.ReadWord();
		m_MusicDataMetaDataEmulationAddresses.m_EmulationAddressOfSequencePointersHighAddress = inReader.GetReadAddress();
		m_MusicData.m_SequencePointersHighAddress = inReader.ReadWord();

		m_MusicData.m_OrderListSize = inReader.ReadWord();
		m_MusicDataMetaDataEmulationAddresses.m_EmulationAddressOfOrderListTrack1Address = inReader.GetReadAddress();
		m_MusicData.m_OrderListTrack1Address = inReader.ReadWord();

		m_MusicData.m_SequenceSize = inReader.ReadWord();
		m_MusicDataMetaDataEmulationAddresses.m_EmulationAddressOfSequence00Address = inReader.GetReadAddress();
		m_MusicData.m_Sequence00Address = inReader.ReadWord();
	}


	void DriverInfo::ParseTableColorRules(Utility::C64FileReader& inReader)
	{
		while (!inReader.HasPassedEnd())
		{
			TableColorRules table_rules = TableColorRules();

			while (!inReader.HasPassedEnd())
			{
				unsigned char first_value = inReader.ReadByte();

				if (first_value == 0xff)
					break;
				if (first_value == 0xfe)
					return;

				TableColorRule table_color_rule;

				table_color_rule.m_EvaluationCellIndex = first_value;
				table_color_rule.m_EvaluationCellMask = inReader.ReadByte();
				table_color_rule.m_EvaluationCellConditionalValue = inReader.ReadByte();
				table_color_rule.m_BackgroundColor = inReader.ReadByte();

				table_rules.m_Rules.push_back(table_color_rule);
			}

			m_TableColorRules.push_back(table_rules);
		}
	}


	void DriverInfo::ParseTableInsDelRules(Utility::C64FileReader& inReader)
	{
		while (!inReader.HasPassedEnd())
		{
			TableInsertDeleteRules table_rules = TableInsertDeleteRules();

			while (!inReader.HasPassedEnd())
			{
				unsigned char first_value = inReader.ReadByte();

				if (first_value == 0xff)
					break;
				if (first_value == 0xfe)
					return;

				TableInsertDeleteRule table_rule;

				table_rule.m_TargetTableID = first_value;
				table_rule.m_TargetCellIndex = inReader.ReadByte();
				table_rule.m_EvaluationCellIndex = inReader.ReadByte();
				table_rule.m_EvaluationCellMask = inReader.ReadByte();
				table_rule.m_EvaluationCellConditionalValue = inReader.ReadByte();

				table_rules.m_Rules.push_back(table_rule);
			}

			m_TableInsDelRules.push_back(table_rules);
		}
	}


	void DriverInfo::ParseTableActionRules(Utility::C64FileReader& inReader)
	{
		while (!inReader.HasPassedEnd())
		{
			TableActionRules table_rules = TableActionRules();

			while (!inReader.HasPassedEnd())
			{
				unsigned char first_value = inReader.ReadByte();

				if (first_value == 0xff)
					break;
				if (first_value == 0xfe)
					return;

				TableActionRule table_rule;

				table_rule.m_ApplicableCell = first_value;
				table_rule.m_TargetTableID = inReader.ReadByte();
				table_rule.m_TargetIndexCell = inReader.ReadByte();
				table_rule.m_TargetIndexMask = inReader.ReadByte();
				table_rule.m_EvaluationCellIndex = inReader.ReadByte();
				table_rule.m_EvaluationCellMask = inReader.ReadByte();
				table_rule.m_EvaluationCellConditionalValue = inReader.ReadByte();

				table_rules.m_Rules.push_back(table_rule);
			}

			m_TableActionRules.push_back(table_rules);
		}
	}


	void DriverInfo::ParseDriverInstrumentDataDescriptor(Utility::C64FileReader& inReader)
	{
		unsigned char instrument_pointer_count = inReader.ReadByte();

		for (int i = 0; i < instrument_pointer_count; ++i)
		{
			InstrumentDataPointerDescription table_pointer_description;

			table_pointer_description.m_TableID = inReader.ReadByte();
			table_pointer_description.m_InstrumentDataPointerPosition = inReader.ReadByte();
			table_pointer_description.m_PointerAndValue = inReader.ReadByte();
			table_pointer_description.m_InstrumentDataConditionalValuePosition = inReader.ReadByte();
			table_pointer_description.m_ConditionValueAndValue = inReader.ReadByte();
			table_pointer_description.m_ConditionEqualityValue = inReader.ReadByte();
			table_pointer_description.m_TableDataType = inReader.ReadByte();
			table_pointer_description.m_TableJumpMarkerValuePosition = inReader.ReadByte();
			table_pointer_description.m_TableJumpMarkerValue = inReader.ReadByte();
			table_pointer_description.m_TableJumpDestinationIndexPosition = inReader.ReadByte();

			m_InstrumentDataDescription.m_InstrumentDataPointerDescriptions.push_back(table_pointer_description);
		}
	}


	bool DriverInfo::ParseAuxilaryData(const Utility::C64File& inFile)
	{
		unsigned short auxilary_data_address = inFile.GetWord(AuxilaryDataPointerAddress);

		if (auxilary_data_address == 0)
			return false;

		Utility::C64FileReader reader = Utility::C64FileReader(inFile, auxilary_data_address);
		m_AuxilaryDataCollection->Load(reader);

		return true;
	}
}