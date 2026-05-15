# SIDFactory II (SF2) File Format Specification

_Version 1.0 - Implementation Agnostic_

## 1. Introduction

The SF2 (SIDFactory II) format, as implemented in SIDFactory II, is a binary
container format for Commodore 64 music. It encapsulates player machine code,
metadata for the editor, and song data (instruments, tables, patterns).

---

## 2. File Topology (Physical Layout)

An SF2 file is a standard C64 PRG file. All multi-byte integers are stored in
**Little-Endian** format.

There are three types of pointers/addresses:
1. **Absolute Memory Address**: A 16-bit value representing a location in the C64 memory map ($0000-$FFFF).
2. **File-Relative Offset**: A position within the PRG file. `Offset = AbsoluteAddress - LoadAddress + 2`.
3. **Table Index**: A single byte index into a row-major or column-major table.

| Offset | Size | Name            | Description                                               |
| :----- | :--- | :-------------- | :-------------------------------------------------------- |
| 0      | 2    | Load Address    | C64 PRG load address (usually where metadata starts).     |
| 2      | 2    | Signature       | Fixed value `0x1337`.                                     |
| 4      | Var  | Metadata Blocks | A sequence of Tag-Length-Value (TLV) blocks.              |
| Var    | 1    | Terminator      | Fixed value `0xFF`.                                       |
| Var    | Var  | Player Code     | The 6502 machine code for the playback engine.            |
| Var    | Var  | Song Data       | Packed tables, music data, and patterns.                  |
| Var    | Var  | Auxiliary Data  | Optional editor-only metadata (if referenced by `$0FFB`). |

### 2.1 Fixed Memory Locations

The following addresses in the C64 memory map (relative to the player's execution context) have special meaning and are used for editor/driver communication:

- **`$0FFB` (2 bytes)**: Pointer to **Auxiliary Data**. If non-zero, it is an **Absolute Memory Address** pointing to the start of the editor-only metadata section.
- **`$0FFD` (3 bytes)**: IRQ Entry point. Typically contains `JMP $xxxx` (Opcode `$4C` followed by an **Absolute Memory Address**) to the player's update routine.

---

## 3. Metadata Block System

Each block follows the structure: `[ID (1 byte)][Size (1 byte)][Data (Size bytes)]`. 
All multi-byte fields are **Little-Endian**. Pointers/Addresses are absolute C64 memory addresses.

### 3.1 Block IDs

| ID   | Name                 | Description                                                 |
| :--- | :------------------- | :---------------------------------------------------------- |
| 0x01 | Descriptor           | Core driver info (version, name, code bounds).              |
| 0x02 | DriverCommon         | Vector table and driver state variable addresses.           |
| 0x03 | DriverTables         | Definitions of tables (Instruments, Commands, etc.).        |
| 0x04 | InstrumentDescriptor | Human-readable labels for instrument cells.                 |
| 0x05 | MusicData            | Pointers and counts for tracks, order lists, and sequences. |
| 0x06 | TableColorRules      | UI coloring rules for the editor.                           |
| 0x07 | TableInsDelRules     | Dependency rules for inserting/deleting table rows.         |
| 0x08 | TableActionRules     | Logic for "Enter" key actions and cross-table navigation.   |
| 0x09 | InstDataDescriptor   | Detailed mapping of instrument fields to sub-tables.        |
| 0xFF | End                  | Signals the end of the metadata section.                    |

### 3.2 Block: Descriptor (0x01)

| Offset | Size | Name       | Description                                                  |
| :----- | :--- | :--------- | :----------------------------------------------------------- |
| 0      | 1    | DriverType | `0x00` for SF2.                                              |
| 1      | 2    | DriverSize | Total size of the driver code.                               |
| 3      | Var  | DriverName | Null-terminated string.                                      |
| Var    | 2    | CodeTop    | Memory address where player code starts (typically `$1000`). |
| Var    | 2    | CodeSize   | Size of the executable code section.                         |
| Var    | 1    | Major      | Major version of the driver.                                 |
| Var    | 1    | Minor      | Minor version of the driver.                                 |
| Var    | 1    | Revision   | Revision version of the driver.                              |

### 3.3 Block: DriverCommon (0x02)

Contains absolute memory addresses for the driver's interface and internal state. All fields are 2 bytes (Word).

| Offset | Name                                | Description                                       |
| :----- | :---------------------------------- | :------------------------------------------------ |
| 0      | InitAddress                         | `JSR` here to initialize the driver.              |
| 2      | StopAddress                         | `JSR` here to stop music and silence SID.         |
| 4      | UpdateAddress                       | `JSR` here every frame (e.g., from IRQ).          |
| 6      | SIDChannelOffsetAddress             | Address of channel offset table (for SID 1,2,3).  |
| 8      | DriverStateAddress                  | Base address of the driver's state variables.     |
| 10     | TickCounterAddress                  | Frame counter per track.                          |
| 12     | OrderListIndexAddress               | Current position in order list per track.         |
| 14     | SequenceIndexAddress                | Current position in sequence per track.           |
| 16     | SequenceInUseAddress                | Status flag if track is active.                   |
| 18     | CurrentSequenceAddress              | Current sequence ID per track.                    |
| 20     | CurrentTransposeAddress             | Current transposition value per track.            |
| 22     | CurrentEventDurationAddress         | Remaining duration for current note.              |
| 24     | NextInstrumentAddress               | Pending instrument change.                        |
| 26     | NextCommandAddress                  | Pending command change.                           |
| 28     | NextNoteAddress                     | Pending note value.                               |
| 30     | NextNoteIsTiedAddress               | Flag if next note is tied.                        |
| 32     | TempoCounterAddress                 | Global tempo counter.                             |
| 34     | TriggerSyncAddress                  | Address to trigger synchronization.               |
| 36     | NoteEventTriggerSyncValue (1 byte)  | Value to write for sync.                          |
| 37     | Reserved (1 byte)                   | Reserved for padding/alignment.                   |
| 38     | Reserved (2 bytes)                  | Reserved for padding/alignment.                   |

### 3.4 Block: DriverTables (0x03)

Defines the structure of various data tables. Consists of a series of table definitions, ending with a `0xFF` byte.

Each definition:
| Offset | Size | Name                     | Description                                         |
| :----- | :--- | :----------------------- | :-------------------------------------------------- |
| 0      | 1    | Type                     | `0x80` Instruments, `0x81` Commands, `0x00` Generic.|
| 1      | 1    | ID                       | Unique identifier for this table.                   |
| 2      | 1    | TextFieldSize            | Bytes reserved for a label (string) per row.        |
| 3      | Var  | Name                     | Null-terminated string.                             |
| Var    | 1    | DataLayout               | `0x00` Row-Major, `0x01` Column-Major.              |
| Var    | 1    | Properties               | Bit 0: Ins/Del, Bit 1: Vert Layout, Bit 2: Cont Mem.|
| Var    | 1    | InsertDeleteRuleID       | ID of rule in block 0x07.                           |
| Var    | 1    | EnterActionRuleID        | ID of action in block 0x08.                         |
| Var    | 1    | ColorRuleID              | ID of coloring rule in block 0x06.                  |
| Var    | 2    | Address                  | Absolute C64 memory address of the table data.      |
| Var    | 2    | ColumnCount              | Number of columns.                                  |
| Var    | 2    | RowCount                 | Number of rows.                                     |
| Var    | 1    | VisibleRowCount          | Editor hint for UI.                                 |

### 3.5 Block: InstrumentDescriptor (0x04)

Provides labels for columns in the instrument table.

| Offset | Size | Name            | Description                             |
| :----- | :--- | :-------------- | :-------------------------------------- |
| 0      | 1    | DescriptorCount | Number of labels.                       |
| 1      | Var  | Labels          | Sequence of null-terminated strings.    |

### 3.6 Block: MusicData (0x05)

| Offset | Size | Name                          | Description                                         |
| :----- | :--- | :---------------------------- | :-------------------------------------------------- |
| 0      | 1    | TrackCount                    | Typically 3.                                        |
| 1      | 2    | OrderListLoAddrPointer        | Memory address holding Lo-bytes of order list ptrs. |
| 3      | 2    | OrderListHiAddrPointer        | Memory address holding Hi-bytes of order list ptrs. |
| 5      | 1    | SequenceCount                 | Total number of sequences.                          |
| 6      | 2    | SequenceLoAddrPointer         | Memory address holding Lo-bytes of sequence ptrs.   |
| 8      | 2    | SequenceHiAddrPointer         | Memory address holding Hi-bytes of sequence ptrs.   |
| 10     | 2    | OrderListSize                 | Max bytes per order list.                           |
| 12     | 2    | OrderListTrack1Address        | Absolute address of the first order list.           |
| 14     | 2    | SequenceSize                  | Max bytes per sequence pattern.                     |
| 16     | 2    | Sequence00Address             | Absolute address of the first sequence pattern.     |

### 3.7 Block: TableColorRules (0x06)

Sequence of rule groups. Each group ends with `0xFF`. The entire block ends with `0xFE`.

Each rule in a group:
| Offset | Size | Name                      | Description                                         |
| :----- | :--- | :------------------------ | :-------------------------------------------------- |
| 0      | 1    | EvaluationCellIndex       | Column index to check.                              |
| 1      | 1    | EvaluationCellMask        | Bitmask to apply before comparison.                 |
| 2      | 1    | EvaluationCellCondValue   | Value to compare against.                           |
| 3      | 1    | BackgroundColor           | C64 color index for the row.                        |

### 3.8 Block: TableInsDelRules (0x07)

Sequence of rule groups. Structure same as 3.7.

Each rule in a group:
| Offset | Size | Name                      | Description                                         |
| :----- | :--- | :------------------------ | :-------------------------------------------------- |
| 0      | 1    | TargetTableID             | Table to modify.                                    |
| 1      | 1    | TargetCellIndex           | Column in target table.                             |
| 2      | 1    | EvaluationCellIndex       | Source column index.                                |
| 3      | 1    | EvaluationCellMask        | Mask for source column.                             |
| 4      | 1    | EvaluationCellCondValue   | Condition for source column.                        |

### 3.9 Block: TableActionRules (0x08)

Sequence of rule groups. Structure same as 3.7.

Each rule in a group:
| Offset | Size | Name                      | Description                                         |
| :----- | :--- | :------------------------ | :-------------------------------------------------- |
| 0      | 1    | ApplicableCell            | Column where "Enter" triggers this action.          |
| 1      | 1    | TargetTableID             | Table to navigate to.                               |
| 2      | 1    | TargetIndexCell           | Column providing the index for navigation.          |
| 3      | 1    | TargetIndexMask           | Mask for navigation index.                          |
| 4      | 1    | EvaluationCellIndex       | Condition column.                                   |
| 5      | 1    | EvaluationCellMask        | Condition mask.                                     |
| 6      | 1    | EvaluationCellCondValue   | Condition value.                                    |

### 3.10 Block: InstDataDescriptor (0x09)

Maps instrument data fields to sub-tables (Wave, Pulse, etc.).

| Offset | Size | Name            | Description                             |
| :----- | :--- | :-------------- | :-------------------------------------- |
| 0      | 1    | PointerCount    | Number of pointer descriptions.         |

Each pointer description (10 bytes):
| Offset | Size | Name                      | Description                                         |
| :----- | :--- | :------------------------ | :-------------------------------------------------- |
| 0      | 1    | TableID                   | Targeted sub-table.                                 |
| 1      | 1    | InstDataPointerPos        | Row index in instrument table for pointer value.    |
| 2      | 1    | PointerAndValue           | Mask for pointer value.                             |
| 3      | 1    | InstDataCondPos           | Row index for condition check.                      |
| 4      | 1    | ConditionAndValue         | Mask for condition.                                 |
| 5      | 1    | ConditionEqualityValue    | Value required for pointer to be active.            |
| 6      | 1    | TableDataType             | `0x00` Single entry, `0x01` Looping with jump.      |
| 7      | 1    | JumpMarkerPos             | Column in sub-table for jump marker check.          |
| 8      | 1    | JumpMarkerValue           | Value identifying a jump (e.g., `0x7F`).            |
| 9      | 1    | JumpDestIndexPos          | Column in sub-table for jump destination.           |

### 3.11 Block: End (0xFF)

Terminates the metadata section. Value: `0xFF`. No size or data.

---

## 4. Song Data Specification

### 4.1 Tables (Block 0x03)

Tables can be stored in **Row-Major** or **Column-Major** layout.

- **Column-Major**: All values for Column 0 are stored sequentially, then Column 1, etc. Used extensively in drivers for indexing via `LDX index / LDA table_col0,x`.
- **Row-Major**: All values for Row 0 are stored, then Row 1, etc.

### 4.2 Music Data (Block 0x05)

This block defines the "roots" for all music content.

- **Tracks**: The number of independent SID channels (typically 3).
- **Order List Track 1 Address**: Absolute address of the first order list.
- **Order List Size**: Fixed size allocated for each track's order list in memory.
- **Sequence 00 Address**: Absolute address of the first sequence pattern.
- **Sequence Size**: Fixed size allocated for each sequence pattern in memory.

### 4.3 Data Formats

- **Order List**: A sequence of bytes ending in `0xFE` (Stop) or `0xFF [Index]` (Loop to Index).
- **Sequence**: A sequence of note tokens and markers ending in `0x7F`.
  - `$00 - $7D`: Note values.
  - `$7E`: Gate ON.
  - `$7F`: End of Sequence.
  - `$80 - $8F`: Duration/Gate tokens.
  - `$90 - $9F`: Tie-note markers.
  - `$A0 - $BF`: Instrument change (Index = `Value & 0x1F`).
  - `$C0 - $FF`: Command change (Index = `Value & 0x3F`).

### 4.4 Table Logic & Jump Commands

Tables (Wave, Pulse, Filter, Arp) can contain control flow logic, typically used for looping or jumping to a different row within the same table. This is configured in the **InstDataDescriptor (Block 0x09)**.

- **Standard Entry**: A row containing data to be processed by the driver.
- **Jump Command**: A special row identifying a change in control flow.

#### 4.4.1 Jump Execution Logic
A tool processing a table should use the following logic (from Block 0x09):

1. Read a row from the table.
2. Check if `TableDataType == 0x01` (Looping/Jump enabled).
3. Compare the byte at `JumpMarkerPos` (within the current row) with `JumpMarkerValue`.
4. If they match:
   - This row is a **Jump Command**.
   - The new table index is found at `JumpDestIndexPos` (within the same row).
   - Continue processing from the new index.
5. If they do not match:
   - This is a **Standard Entry**.
   - Increment current row index and continue next frame.

_Common Convention_: Many drivers use `0x7F` as the `JumpMarkerValue` and store the destination row index in the following byte (if `JumpDestIndexPos` points there).

---

## 5. Dependency Graph & Optimization

To safely de-duplicate or prune data, a tool must follow this reachability chain:

1. **MusicData** defines `N` tracks.
2. Each track has an **Order List**.
3. Each **Order List** contains indices to **Sequences**.
4. Each **Sequence** contains indices to **Instruments** and **Commands**.
5. Each **Instrument** (via **InstDataDescriptor**) contains indices to sub-tables (Wave, Pulse, Filter, Arp).
6. **Sub-tables** may contain "Jump" commands (`0x7F`) referencing other indices within the same table.

---

## 6. Relocation & Packing Logic

### 6.1 Address Relocation

When the `CodeTop` in the file differs from the intended `LoadAddress`, the following relocation must occur to the machine code and data pointers:

1. **Absolute Relocation**:
   Scan the "Player Code" section for 3-byte opcodes using absolute addressing modes:
   `$0D, $0E, $1D, $1E, $20, $2D, $2E, $40, $4D, $4E, $6D, $6E, $8D, $8E, $AD, $AE, $CD, $CE, $ED, $EE`.
   If the 16-bit operand (bytes 1-2) falls within the original code/data range:
   `NewOperand = OldOperand + (NewLoadAddress - OriginalCodeTop)`.

2. **Zero-Page Relocation**:
   Scan for 2-byte opcodes with ZP addressing.
   Shift the operand by `ZP_Delta` if the driver supports a relocatable Zero-Page base.

3. **Metadata Pointer Update**:
   All 16-bit addresses in Metadata Blocks (DriverCommon, DriverTables, MusicData, InstDataDescriptor) must be updated:
   `NewAddress = OldAddress + (NewLoadAddress - OriginalCodeTop)`.

### 6.2 Packing

When writing the file to disk:

1. **Serialize**: Concatenate Player Code + Table Data + Music Data + Patterns.
2. **Update Pointers**: Update all pointers in Metadata Blocks to match the final absolute memory addresses they will occupy when loaded.
3. **Calculate Offset**: When a tool needs to find data within the file: `FileOffset = AbsoluteAddress - LoadAddress + 2`.
   - The `+ 2` accounts for the PRG Load Address (bytes 0-1 of the file).
   - `LoadAddress` is the value stored in bytes 0-1 of the PRG file.

---

## 7. Auxiliary Data (Editor Metadata)

Stored at the end of the file. Structured as a series of data chunks (Songs, Authors, Colors, etc.) used purely by the SIDFactory II UI and ignored by the C64 player code.
