# SIDFactory II (SF2) File Format Specification

_Version 1.1 - Implementation Agnostic_

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
| Var    | Var  | Auxiliary Data  | Optional editor-only metadata (see Section 7).            |

### 2.1 Fixed Memory Locations

The following are **absolute fixed C64 addresses** (not relative to load address). They reside in the zero-page / low-memory area and are patched by the editor at save time:

- **`$0FFB` (2 bytes, Little-Endian)**: Pointer to **Auxiliary Data**. If non-zero, it is an absolute C64 memory address pointing to the start of the editor-only metadata section. This value is written by the editor as `InitAddress - 5` (where `InitAddress` is from Block 0x02).
- **`$0FFD` (3 bytes)**: IRQ Entry point. Contains `JMP $xxxx` (opcode `$4C` followed by a 16-bit absolute address in Little-Endian) to the player's update routine. This is patched by the editor to point to the correct IRQ handler after packing.

These locations are placed by the driver source at `* = $0ffb` and are always at these absolute addresses regardless of where the driver code loads.

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
| 1      | 2    | DriverSize | Total size of the driver code (bottom - top).                |
| 3      | Var  | DriverName | Null-terminated string (PETSCII, `@` = line break).          |
| Var    | 2    | CodeTop    | Absolute address where player code starts (typically `$1000`). |
| Var+2  | 2    | CodeSize   | Size of the executable code section.                         |
| Var+4  | 1    | Major      | Major version of the driver.                                 |
| Var+5  | 1    | Minor      | Minor version of the driver.                                 |
| Var+6  | 1    | Revision   | Revision version of the driver (optional; may be absent).    |

**Note**: The `Revision` byte is optional. If the block ends before this byte, it defaults to `0`.

### 3.3 Block: DriverCommon (0x02)

Contains absolute memory addresses for the driver's interface and internal state. All fields are 2 bytes (Word) unless noted.

| Offset | Size | Name                               | Description                                      |
| :----- | :--- | :--------------------------------- | :----------------------------------------------- |
| 0      | 2    | InitAddress                        | `JSR` here to initialize the driver.             |
| 2      | 2    | StopAddress                        | `JSR` here to stop music and silence SID.        |
| 4      | 2    | UpdateAddress                      | `JSR` here every frame (e.g., from IRQ).         |
| 6      | 2    | SIDChannelOffsetAddress            | Address of channel offset table (for SID 1,2,3). |
| 8      | 2    | DriverStateAddress                 | Base address of the driver's state variables.    |
| 10     | 2    | TickCounterAddress                 | Frame counter per track.                         |
| 12     | 2    | OrderListIndexAddress              | Current position in order list per track.        |
| 14     | 2    | SequenceIndexAddress               | Current position in sequence per track.          |
| 16     | 2    | SequenceInUseAddress               | Status flag if track is active.                  |
| 18     | 2    | CurrentSequenceAddress             | Current sequence ID per track.                   |
| 20     | 2    | CurrentTransposeAddress            | Current transposition value per track.           |
| 22     | 2    | CurrentEventDurationAddress        | Remaining duration for current note.             |
| 24     | 2    | NextInstrumentAddress              | Pending instrument change.                       |
| 26     | 2    | NextCommandAddress                 | Pending command change.                          |
| 28     | 2    | NextNoteAddress                    | Pending note value.                              |
| 30     | 2    | NextNoteIsTiedAddress              | Flag if next note is tied.                       |
| 32     | 2    | TempoCounterAddress                | Global tempo counter.                            |
| 34     | 2    | TriggerSyncAddress                 | Address written to trigger synchronization.      |
| 36     | 1    | NoteEventTriggerSyncValue          | Value written to TriggerSyncAddress to trigger sync. |
| 37     | 1    | Reserved                           | Reserved for padding/alignment.                  |
| 38     | 2    | Reserved                           | Reserved for padding/alignment.                  |

### 3.4 Block: DriverTables (0x03)

Defines the structure of various data tables. Consists of a series of table definitions, ending with a `0xFF` byte.

Each definition:

| Offset | Size | Name               | Description                                          |
| :----- | :--- | :----------------- | :--------------------------------------------------- |
| 0      | 1    | Type               | `0x80` Instruments, `0x81` Commands, `0x00` Generic. |
| 1      | 1    | ID                 | Unique identifier for this table.                    |
| 2      | 1    | TextFieldSize      | Bytes reserved for a label (string) per row.         |
| 3      | Var  | Name               | Null-terminated string (PETSCII, `@` = line break).  |
| Var    | 1    | DataLayout         | `0x00` Row-Major, `0x01` Column-Major.               |
| Var+1  | 1    | Properties         | Bit 0: EnableInsDel, Bit 1: LayoutVertically, Bit 2: IndexAsContinuousMemory. |
| Var+2  | 1    | InsertDeleteRuleID | ID of rule in block 0x07. `$FF` = none.              |
| Var+3  | 1    | EnterActionRuleID  | ID of action in block 0x08. `$FF` = none.            |
| Var+4  | 1    | ColorRuleID        | ID of coloring rule in block 0x06. `$FF` = none.     |
| Var+5  | 2    | Address            | Absolute C64 memory address of the table data.       |
| Var+7  | 2    | ColumnCount        | Number of columns (bytes per row).                   |
| Var+9  | 2    | RowCount           | Number of rows (max entries per column).             |
| Var+11 | 1    | VisibleRowCount    | Editor hint for UI scroll window size.               |

**Properties bitfield**:
- Bit 0 (`0x01`): `EnableInsertDelete` — table supports row insert/delete editing.
- Bit 1 (`0x02`): `LayoutAddVertically` — table is laid out vertically relative to the previous table.
- Bit 2 (`0x04`): `IndexAsContinuousMemory` — index the table as if it were continuous memory.

### 3.5 Block: InstrumentDescriptor (0x04)

Provides labels for columns in the instrument table.

| Offset | Size | Name            | Description                          |
| :----- | :--- | :-------------- | :----------------------------------- |
| 0      | 1    | DescriptorCount | Number of labels.                    |
| 1      | Var  | Labels          | Sequence of null-terminated strings (PETSCII, `@` = line break). |

### 3.6 Block: MusicData (0x05)

| Offset | Size | Name                   | Description                                         |
| :----- | :--- | :--------------------- | :-------------------------------------------------- |
| 0      | 1    | TrackCount             | Number of tracks (typically 3).                     |
| 1      | 2    | OrderListLoAddrPointer | Absolute address of the array of low-bytes of order list pointers (one per track). |
| 3      | 2    | OrderListHiAddrPointer | Absolute address of the array of high-bytes of order list pointers (one per track). |
| 5      | 1    | SequenceCount          | Total number of sequences allocated.                |
| 6      | 2    | SequenceLoAddrPointer  | Absolute address of the array of low-bytes of sequence pointers. |
| 8      | 2    | SequenceHiAddrPointer  | Absolute address of the array of high-bytes of sequence pointers. |
| 10     | 2    | OrderListSize          | Maximum buffer size allocated per track's order list in C64 memory. The actual packed order list length is determined by scanning for the end marker (`0xFE` or `0xFF`). |
| 12     | 2    | OrderListTrack1Address | Absolute address of the first track's order list buffer. Subsequent tracks follow at `+ OrderListSize` intervals. |
| 14     | 2    | SequenceSize           | Maximum buffer size allocated per sequence pattern in C64 memory. |
| 16     | 2    | Sequence00Address      | Absolute address of the first sequence pattern buffer. Subsequent sequences follow at `+ SequenceSize` intervals. |

### 3.7 Block: TableColorRules (0x06)

A sequence of rule groups. Each group applies to one table (matched by rule group order to table definition order). The block is terminated when the first byte of a rule is `0xFE`.

**Parsing**: Read bytes one at a time. The first byte of each rule serves double duty:
- If `0xFF`: end of the current rule group. Start a new group (if more data follows).
- If `0xFE`: end of all rules. Stop parsing.
- Otherwise: this is `EvaluationCellIndex`, and the next 3 bytes follow.

Each rule (4 bytes):

| Offset | Size | Name                    | Description                         |
| :----- | :--- | :---------------------- | :---------------------------------- |
| 0      | 1    | EvaluationCellIndex     | Column index to check.              |
| 1      | 1    | EvaluationCellMask      | Bitmask to apply before comparison. |
| 2      | 1    | EvaluationCellCondValue | Value to compare against.           |
| 3      | 1    | BackgroundColor         | C64 color index for the row.        |

A row is colored when `(row[EvaluationCellIndex] & EvaluationCellMask) == EvaluationCellCondValue`.

### 3.8 Block: TableInsDelRules (0x07)

A sequence of rule groups. Terminated the same way as Block 0x06 (`0xFF` = end of group, `0xFE` = end of all rules).

Each rule (5 bytes):

| Offset | Size | Name                    | Description                  |
| :----- | :--- | :---------------------- | :--------------------------- |
| 0      | 1    | TargetTableID           | Table ID to modify.          |
| 1      | 1    | TargetCellIndex         | Column index in target table. |
| 2      | 1    | EvaluationCellIndex     | Source column index.         |
| 3      | 1    | EvaluationCellMask      | Mask for source column.      |
| 4      | 1    | EvaluationCellCondValue | Condition for source column. |

When a row is inserted or deleted in the source table, this rule specifies which cell in which target table should be adjusted. The condition determines when the adjustment applies.

### 3.9 Block: TableActionRules (0x08)

A sequence of rule groups. Terminated the same way as Block 0x06 (`0xFF` = end of group, `0xFE` = end of all rules).

Each rule (7 bytes):

| Offset | Size | Name                    | Description                                |
| :----- | :--- | :---------------------- | :----------------------------------------- |
| 0      | 1    | ApplicableCell          | Column where "Enter" triggers this action. `$80` = any column. |
| 1      | 1    | TargetTableID           | Table ID to navigate to. `$FF` = self (same table). |
| 2      | 1    | TargetIndexCell         | Column providing the index for navigation. |
| 3      | 1    | TargetIndexMask         | Mask applied to the navigation index value. |
| 4      | 1    | EvaluationCellIndex     | Condition column.                          |
| 5      | 1    | EvaluationCellMask      | Condition mask.                            |
| 6      | 1    | EvaluationCellCondValue | Condition value.                           |

When the user presses Enter on a cell matching `ApplicableCell`, the editor navigates to `TargetTableID` at the row index given by `table[TargetIndexCell] & TargetIndexMask`, provided the condition `(table[EvaluationCellIndex] & EvaluationCellMask) == EvaluationCellCondValue` is met.

### 3.10 Block: InstDataDescriptor (0x09)

Maps instrument data fields to sub-tables (Wave, Pulse, Filter, Arp, etc.).

| Offset | Size | Name         | Description                     |
| :----- | :--- | :----------- | :------------------------------ |
| 0      | 1    | PointerCount | Number of pointer descriptions. |

Each pointer description (10 bytes):

| Offset | Size | Name                         | Description                                      |
| :----- | :--- | :--------------------------- | :----------------------------------------------- |
| 0      | 1    | TableID                      | Target sub-table ID.                              |
| 1      | 1    | InstDataPointerPosition      | Column index in instrument table holding the pointer/index value. |
| 2      | 1    | PointerAndValue              | Bitmask applied to the pointer value to get the real table index. |
| 3      | 1    | InstDataConditionalValuePosition | Column index in instrument table for the condition check. |
| 4      | 1    | ConditionAndValue            | Bitmask applied to the condition value.          |
| 5      | 1    | ConditionEqualityValue       | The condition is met when `(instrument[InstDataConditionalValuePosition] & ConditionAndValue) == ConditionEqualityValue`. |
| 6      | 1    | TableDataType                | `0x00` = single entry, `0x01` = looping with jump markers. |
| 7      | 1    | TableJumpMarkerValuePosition | Column index in the sub-table to check for the jump marker. |
| 8      | 1    | TableJumpMarkerValue         | Value that identifies a jump row (commonly `0x7F`). |
| 9      | 1    | TableJumpDestinationIndexPosition | Column index in the sub-table holding the jump destination row index. |

### 3.11 Block: End (0xFF)

Terminates the metadata section. Value: `0xFF`. No size or data byte follows.

---

## 4. Song Data Specification

### 4.1 Tables (Block 0x03)

Tables can be stored in **Row-Major** or **Column-Major** layout.

- **Row-Major** (`DataLayout = 0x00`): All values for Row 0 are stored, then Row 1, etc. Byte offset for cell `(row, col)` = `row * ColumnCount + col`.
- **Column-Major** (`DataLayout = 0x01`): All values for Column 0 are stored sequentially, then Column 1, etc. Byte offset for cell `(row, col)` = `col * RowCount + row`. Used extensively in drivers for indexing via `LDX index / LDA table_col0,x`.

### 4.2 Music Data (Block 0x05)

This block defines the "roots" for all music content.

- **Tracks**: The number of independent SID channels (typically 3).
- **Order List Track 1 Address**: Absolute address of the first track's order list buffer. Track N is at `OrderListTrack1Address + (N - 1) * OrderListSize`.
- **Order List Size**: Maximum buffer size allocated per track's order list in C64 memory. This is a scratch-space allocation, not the actual data length. The actual packed order list length is determined by scanning for the end marker.
- **Sequence 00 Address**: Absolute address of the first sequence pattern buffer. Sequence N is at `Sequence00Address + N * SequenceSize`.
- **Sequence Size**: Maximum buffer size allocated per sequence pattern in C64 memory.

### 4.3 Data Formats

#### Order List (Packed Format)

Order lists are stored in C64 memory as a packed byte stream terminated by an end marker:

- Bytes `< 0x80`: **Sequence index** — references a sequence to play.
- Bytes `0x80 - 0xFD`: **Transposition value** — applies to all subsequent sequence entries until another transposition byte or the end marker.
- `0xFE`: **Stop marker** — end of the order list. Playback stops.
- `0xFF [Index]`: **Loop marker** — end of the order list with a loop. `Index` is the byte position within the packed data to loop back to.

The editor internally represents order lists as `(Transposition, SequenceIndex)` pairs and packs/unpacks them to/from this format.

#### Sequence (Packed Format)

Sequences are stored as a packed byte stream terminated by `0x7F`. Parsing is sequential:

| Byte Range | Meaning |
| :--------- | :------ |
| `0x00`     | Note off (gate off). |
| `0x01 - 0x6F` | Note value (MIDI-style note number, 1-111). |
| `0x70 - 0x7D` | Reserved. |
| `0x7E`     | Gate ON (sustain the current note). |
| `0x7F`     | End of sequence. |
| `0x80 - 0x8F` | Duration token. Duration = `Value & 0x0F` (0-15 frames). Applies to the following note. |
| `0x90 - 0x9F` | Tie-note duration token. Duration = `Value & 0x0F`. The following note is tied to the previous one. |
| `0xA0 - 0xBF` | Instrument change. Instrument index = `Value & 0x1F` (0-31). |
| `0xC0 - 0xFF` | Command change. Command index = `Value & 0x3F` (0-63). |

**Parsing order**: When reading a sequence byte stream, process bytes in this priority:
1. If byte `>= 0xC0`: it's a command, consume it, read next byte.
2. If byte `>= 0xA0`: it's an instrument, consume it, read next byte.
3. If byte `>= 0x80`: it's a duration/tie token, extract duration, read next byte for the note.
4. If byte `< 0x80`: it's a note value (or `0x7F` end marker).

Instrument and command values persist across events until changed. Duration tokens only need to be emitted when the duration changes from the previous event.

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
6. **Commands** may contain indices referencing rows in other tables (e.g., Arp, Vibrato, etc.).
7. **Sub-tables** may contain "Jump" commands (`0x7F`) referencing other indices within the same table.

### 5.1 Implicit References

For optimization and pruning, row 0 of tables whose **Name** (from Block 0x03) matches any of the following is always considered implicitly referenced:

- **`init`**: Initial driver state.
- **`speed`**: Default playback speed/tempo.
- **`hr`**: Hard-restart settings.

A tool must never prune row 0 of these tables, even if no explicit reference exists in the song data. This is a naming convention, not a structural property encoded in metadata.

---

## 6. Relocation & Packing Logic

### 6.1 Address Relocation

When the `CodeTop` in the file differs from the intended `LoadAddress`, the following relocation must occur:

1. **Absolute Addressing Relocation**:
   Scan the "Player Code" section (from `CodeTop` to `CodeTop + CodeSize`) instruction by instruction. For each opcode, determine its addressing mode. If the addressing mode is **Absolute** (`am_ABS`), **Absolute,X** (`am_ABX`), **Absolute,Y** (`am_ABY`), or **Indirect** (`am_IND`):
   - Read the 16-bit operand at `address + 1`.
   - If the operand falls in the `$D000-$DFFF` range (SID register space), relocate it using `GetRelocatedVector(operand)` — find which data section contains this address and map it to its new destination address. **Do not add the delta.**
   - For all other operands, relocate as: `NewOperand = GetRelocatedVector(OldOperand) + DestinationAddressDelta`, where `DestinationAddressDelta = NewLoadAddress - OriginalCodeTop`.
   - Write the relocated operand back.

2. **Zero-Page Relocation**:
   Scan for opcodes with Zero-Page addressing modes (`am_ZP`, `am_ZPX`, `am_ZPY`, `am_IZX`, `am_IZY`). For each:
   - Read the zero-page operand byte.
   - Compute: `ZeroPageOffset = operand - OriginalLowestZP`, then `NewOperand = ZeroPageOffset + NewLowestZP`.
   - `OriginalLowestZP` is the lowest zero-page address found by scanning the driver code. `NewLowestZP` is the target zero-page base.
   - Write the relocated operand byte back.

3. **Metadata Pointer Update**:
   All 16-bit addresses in Metadata Blocks (DriverCommon, DriverTables, MusicData, InstDataDescriptor) must be updated:
   `NewAddress = OldAddress + (NewLoadAddress - OriginalCodeTop)`.

### 6.2 Packing

When writing the file to disk:

1. **Collect data sections**: Gather all tables (only used rows), order list pointers, sequence pointers, order lists, and sequences.
2. **Compute destination addresses**: Pack data sections contiguously after the driver code.
3. **Copy data**: Write driver code followed by all data sections to the output buffer.
4. **Adjust pointers**: Update order list pointer arrays and sequence pointer arrays to point to the new packed addresses.
5. **Process driver code**: Apply absolute and zero-page relocation as described in Section 6.1.
6. **Multi-song patch**: If more than one song is present, append a patch code block that copies order list pointers for the selected song into the driver's state at init time.
7. **Write vectors**: Patch the IRQ vector at `$0FFD` and auxiliary data pointer at `$0FFB`.
8. **Calculate file offset**: `FileOffset = AbsoluteAddress - LoadAddress + 2`.
   - The `+ 2` accounts for the PRG Load Address (bytes 0-1 of the file).
   - `LoadAddress` is the value stored in bytes 0-1 of the PRG file.

---

## 7. Auxiliary Data (Editor Metadata)

Auxiliary data is stored at the end of the file. It contains editor-only metadata that is ignored by the C64 player code. The address of this section is stored at `$0FFB` (and can also be computed as `InitAddress - 5`).

### 7.1 Chunk Format

Auxiliary data is a sequence of chunks. Each chunk has a header followed by data:

| Offset | Size | Name       | Description                              |
| :----- | :--- | :--------- | :--------------------------------------- |
| 0      | 1    | Type       | Chunk type identifier (see 7.2).         |
| 1      | 2    | Version    | Data format version (Little-Endian).     |
| 3      | 2    | DataSize   | Size of the data payload (Little-Endian). |
| 5      | Var  | Data       | Chunk-specific data (DataSize bytes).    |

Chunks are written in this fixed order: PlayMarkers, HardwarePreferences, EditingPreferences, TableText, Songs. The sequence is terminated by a chunk with `Type = 0` (Undefined), `Version = 0`, `DataSize = 0`, and no data payload.

### 7.2 Chunk Types

| Type ID | Name                  | Description                          |
| :------ | :-------------------- | :----------------------------------- |
| 0       | Undefined             | Terminator chunk.                    |
| 1       | EditingPreferences    | Note notation and display settings.  |
| 2       | HardwarePreferences   | SID model and video region.          |
| 3       | PlayMarkers           | Playback position markers.           |
| 4       | TableText             | User-defined text labels for tables. |
| 5       | Songs                 | Multi-song metadata (names, etc.).   |

### 7.3 Chunk: EditingPreferences (Type 1)

Version: 1

| Offset | Size | Name                      | Description                              |
| :----- | :--- | :------------------------ | :--------------------------------------- |
| 0      | 1    | NotationMode              | `0` = Sharp, `1` = Flat.                |
| 1      | 1    | EventPosHighlightOffset   | Offset for event position highlighting.  |
| 2      | 1    | EventPosHighlightInterval | Interval for event position highlighting. |

### 7.4 Chunk: HardwarePreferences (Type 2)

Version: 1

| Offset | Size | Name       | Description              |
| :----- | :--- | :--------- | :----------------------- |
| 0      | 1    | SIDModel   | `0` = MOS6581, `1` = MOS8580. |
| 1      | 1    | Region     | `0` = PAL, `1` = NTSC.   |

### 7.5 Chunk: PlayMarkers (Type 3)

Version: 2

| Offset | Size | Name          | Description                              |
| :----- | :--- | :------------ | :--------------------------------------- |
| 0      | 1    | LayerCount    | Number of marker layers.                 |

For each layer:

| Offset | Size | Name               | Description                            |
| :----- | :--- | :----------------- | :------------------------------------- |
| 0      | 1    | MarkerCount       | Number of markers in this layer.       |
| 1      | Var  | EventPositions    | `MarkerCount` × 4 bytes, each a signed 32-bit integer (Little-Endian) representing event positions. |

### 7.6 Chunk: TableText (Type 4)

Version: 2

| Offset | Size | Name              | Description                            |
| :----- | :--- | :---------------- | :------------------------------------- |
| 0      | 1    | TableCount        | Number of table text buckets.          |

For each table bucket:

| Offset | Size | Name              | Description                            |
| :----- | :--- | :---------------- | :------------------------------------- |
| 0      | 4    | TableID           | Table ID (Little-Endian 32-bit int).   |
| 4      | 2    | LayerCount        | Number of text layers for this table.  |

For each layer:

| Offset | Size | Name              | Description                            |
| :----- | :--- | :---------------- | :------------------------------------- |
| 0      | 2    | EntryCount        | Number of text entries in this layer.  |

For each entry:

| Offset | Size | Name       | Description                            |
| :----- | :--- | :--------- | :------------------------------------- |
| 0      | 1    | Length     | String length (0-255).                 |
| 1      | Var    | Text       | `Length` bytes of text (no null terminator). |

### 7.7 Chunk: Songs (Type 5)

Version: 2

| Offset | Size | Name          | Description                            |
| :----- | :--- | :------------ | :------------------------------------- |
| 0      | 1    | SongCount     | Number of songs.                       |
| 1      | 1    | SelectedSong  | Index of the currently selected song.  |

For each song:

| Offset | Size | Name       | Description                            |
| :----- | :--- | :--------- | :------------------------------------- |
| 0      | 1    | Length     | Song name string length (0-255).       |
| 1      | Var    | SongName   | `Length` bytes of text (no null terminator). |

**Version 1 compatibility**: Version 1 of this chunk has the same `SongCount` and `SelectedSong` fields but omits song names (no string data follows).
