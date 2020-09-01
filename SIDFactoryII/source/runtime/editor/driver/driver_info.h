#pragma once

#include <string>
#include <vector>
#include <memory>

namespace Utility
{
	class C64File;
	class C64FileReader;
}
namespace Editor
{
	class IDriverArchitecture;
	class AuxilaryDataCollection;

	class DriverInfo final
	{
		static const unsigned short ExpectedFileIDNumber = 0x1337;
		static const unsigned short AuxilaryDataPointerAddress = 0x0ffb;

	public:
		enum TableType : unsigned char
		{
			Generic = 0x00,
			Instruments = 0x80,
			Commands = 0x81
		};


		enum HeaderBlockID : unsigned int
		{
			ID_Descriptor = 1,
			ID_DriverCommon = 2,
			ID_DriverTables = 3,
			ID_DriverInstrumentDesciptor = 4,
			ID_MusicData = 5,
			ID_TableColorRules = 6,
			ID_TableInsertDeleteRules = 7,
			ID_TableActionRules = 8,
			ID_DriverInstrumentDataDescriptor = 9,

			_IDBlock_Count = 10,

			ID_End = 0xff
		};

		struct Descriptor
		{
			unsigned char m_DriverType;
			unsigned short m_DriverSize;
			std::string m_DriverName;
			unsigned short m_DriverCodeTop;
			unsigned short m_DriverCodeSize;
			unsigned char m_DriverVersionMajor;
			unsigned char m_DriverVersionMinor;
		};


		struct DriverCommon
		{
			unsigned short m_InitAddress;
			unsigned short m_StopAddress;
			unsigned short m_UpdateAddress;

			unsigned short m_SIDChannelOffsetAddress;

			unsigned short m_DriverStateAddress;

			unsigned short m_TickCounterAddress;
			unsigned short m_OrderListIndexAddress;
			unsigned short m_SequenceIndexAddress;				// Index into the sequence

			unsigned short m_SequenceInUseAddress;
			unsigned short m_CurrentSequenceAddress;
			unsigned short m_CurrentTransposeAddress;
			unsigned short m_CurrentSequenceEventDurationAddress;
			unsigned short m_NextInstrumentAddress;
			unsigned short m_NextCommandAddress;
			unsigned short m_NextNoteAddress;
			unsigned short m_NextNoteIsTiedAddress;
			unsigned short m_TempoCounterAddress;
			unsigned short m_TriggerSyncAddress;
			unsigned char m_NoteEventTriggerSyncValue;
			unsigned char m_ReservedByte;
			unsigned short m_ReservedWord;
		};


		struct MusicData
		{
			unsigned char m_TrackCount;
			unsigned short m_TrackOrderListPointersLowAddress;
			unsigned short m_TrackOrderListPointersHighAddress;

			unsigned char m_SequenceCount;
			unsigned short m_SequencePointersLowAddress;
			unsigned short m_SequencePointersHighAddress;

			unsigned short m_OrderListSize;
			unsigned short m_OrderListTrack1Address;

			unsigned short m_SequenceSize;
			unsigned short m_Sequence00Address;
		};


		struct TableDefinition
		{
			enum DataLayout : unsigned char
			{
				RowMajor = 0,
				ColumnMajor = 1
			};

			enum Properties : unsigned char
			{
				EnableInsertDelete = 0x01,
				LayoutAddVertically = 0x02,
				IndexAsContiuousMemory = 0x04
			};

			unsigned char m_Type;						// Type of table
			unsigned char m_ID;							// Identifier for rule targeting
			unsigned char m_TextFieldSize;				// Size of the text field added to the table
			std::string m_Name;							// The readable name of the table

			DataLayout m_DataLayout;					// Data layout in the emulated memory
			
			bool m_PropertyEnabledInsertDelete;			// Support of insert and delete editing
			bool m_PropertyLayoutVertically;			// Layout vertically relative to last table added
			bool m_PropertyIndexAsContinuousMemory;		// Index the table as if it were continuous memory

			unsigned char m_InsertDeleteRuleID;			// ID of rule to modify other tables when inserting or deleting
			unsigned char m_EnterActionRuleID;			// ID of action execution when hitting enter while editing this table
			unsigned char m_ColorRuleID;				// ID of the color rules group to apply

			unsigned short m_Address;					// Address of data
			unsigned short m_ColumnCount;				// Number of columns in this table
			unsigned short m_RowCount;					// Number of rows in this table

			unsigned char m_VisibleRowCount;			// Number of visible rows on the table
		};

		struct InstrumentDescriptor
		{
			std::vector<std::string> m_CellDescription;
		};

		struct TableColorRule
		{
			unsigned char m_EvaluationCellIndex;
			unsigned char m_EvaluationCellMask;
			unsigned char m_EvaluationCellConditionalValue;
			unsigned char m_BackgroundColor;
		};

		struct TableColorRules
		{
			std::vector<TableColorRule> m_Rules;
		};

		struct TableInsertDeleteRule
		{
			unsigned char m_TargetTableID;
			unsigned char m_TargetCellIndex;
			unsigned char m_EvaluationCellIndex;
			unsigned char m_EvaluationCellMask;
			unsigned char m_EvaluationCellConditionalValue;
		};

		struct TableInsertDeleteRules
		{
			std::vector<TableInsertDeleteRule> m_Rules;
		};

		struct TableActionRule
		{
			unsigned char m_ApplicableCell;
			unsigned char m_TargetIndexCell;
			unsigned char m_TargetIndexMask;
			unsigned char m_TargetTableID;

			unsigned char m_EvaluationCellIndex;
			unsigned char m_EvaluationCellMask;
			unsigned char m_EvaluationCellConditionalValue;
		};

		struct TableActionRules
		{
			std::vector<TableActionRule> m_Rules;
		};

		struct InstrumentDataPointerDescription
		{
			unsigned char m_TableID;								// ID of the table being pointed to
			unsigned char m_InstrumentDataPointerPosition;			// Position of the data entry holding the index into the table
			unsigned char m_PointerAndValue;						// Value to and with the pointer value to get the real pointer into the table
			unsigned char m_InstrumentDataConditionalValuePosition;	// Position of the data entry holding a conditional value for the table pointer to be included
			unsigned char m_ConditionValueAndValue;					// Value to and to the condition value
			unsigned char m_ConditionEqualityValue;					// The final result of the conditional value and with the and value should equal this for the data pointer to be included.
			unsigned char m_TableDataType;							// Type 0: single entry, Type 1: looping data with jump markers
			unsigned char m_TableJumpMarkerValuePosition;			// Position in the table to look for the jump marker value
			unsigned char m_TableJumpMarkerValue;					// Jump marker value
			unsigned char m_TableJumpDestinationIndexPosition;		// Position in the table to look for the destination index when jumping
		};

		struct InstrumentDataDescription
		{
			std::vector<InstrumentDataPointerDescription> m_InstrumentDataPointerDescriptions;
		};

		DriverInfo();
		~DriverInfo();

		void Parse(const Utility::C64File& inFile);
		bool IsValid() const;
		bool IsParticalyValid() const;

		const Descriptor& GetDescriptor() const;
		const DriverCommon& GetDriverCommon() const;
		const MusicData& GetMusicData() const;
		const std::vector<TableDefinition>& GetTableDefinitions() const;
		const std::vector<TableColorRules>& GetTableColorRules() const;
		const std::vector<TableInsertDeleteRules>& GetTableInsertDeleteRules() const;
		const std::vector<TableActionRules>& GetTableActionRules() const;
		const InstrumentDataDescription& GetInstrumentDataDescription() const;

		unsigned short GetTopAddress() const;

		IDriverArchitecture* const GetDriverArchitecture() const;

		bool HasFoundHeaderBlock(HeaderBlockID inBlockID) const;
		bool HasParsedHeaderBlock(HeaderBlockID inBlockID) const;

		bool HasEditData() const;

		AuxilaryDataCollection& GetAuxilaryDataCollection();
		const AuxilaryDataCollection& GetAuxilaryDataCollection() const;

	private:
		bool HasParsedRequiredBlocks() const;

		void SetHasFoundHeaderBlock(HeaderBlockID inBlockID);
		void SetHasParsedHeaderBlock(HeaderBlockID inBlockID);

		bool ParseHeader(const Utility::C64File& inFile);

		void ParseDescriptor(Utility::C64FileReader& inReader);
		void ParseDriverCommon(Utility::C64FileReader& inReader);
		void ParseDriverTables(Utility::C64FileReader& inReader);
		void ParseDriverInstrumentDescriptor(Utility::C64FileReader& inReader);
		void ParseMusicData(Utility::C64FileReader& inReader);
		void ParseTableColorRules(Utility::C64FileReader& inReader);
		void ParseTableInsDelRules(Utility::C64FileReader& inReader);
		void ParseTableActionRules(Utility::C64FileReader& inReader);
		void ParseDriverInstrumentDataDescriptor(Utility::C64FileReader& inReader);

		bool ParseAuxilaryData(const Utility::C64File& inFile);

		bool m_IsValid;
		bool m_FoundRequiredTableInstruments;
		bool m_FoundRequiredTableCommands;
		bool m_HasEditData;

		unsigned short m_TopAddress;

		unsigned int m_FoundDescriptorBlocks;
		unsigned int m_ParsedDescriptorBlocks;

		std::unique_ptr<IDriverArchitecture> m_DriverArchitecture;

		Descriptor m_Descriptor;
		DriverCommon m_DriverCommon;
		MusicData m_MusicData;

		std::vector<TableDefinition> m_TableDefinitions;
		std::vector<TableColorRules> m_TableColorRules;
		std::vector<TableInsertDeleteRules> m_TableInsDelRules;
		std::vector<TableActionRules> m_TableActionRules;

		InstrumentDataDescription m_InstrumentDataDescription;

		std::unique_ptr<AuxilaryDataCollection> m_AuxilaryDataCollection;

		InstrumentDescriptor m_InstrumentDescriptor;
	};
}
