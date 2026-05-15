# SIDFactory II (SF2) File Format Specification

_Version 1.0 - Implementation Agnostic_

## 1. Introduction

The SF2 (SIDFactory II) format, as implemented in SIDFactory II, is a binary
container format for Commodore 64 music. It encapsulates player machine code,
metadata for the editor, and song data (instruments, tables, patterns).

---

## 2. File Topology (Physical Layout)

An SF2 file is a standard C64 PRG file. All multi-byte integers are stored in
**Little-Endian** format unless otherwise specified.

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

The following addresses in the C64 memory map (relative to the player's execution context) have special meaning:

- **`$0FFB` (2 bytes)**: Pointer to **Auxiliary Data**. If non-zero, it points to the start of the editor-only metadata section.
- **`$0FFD` (3 bytes)**: IRQ Entry point. Typically contains `JMP $xxxx` to the player's update routine.

---

## 3. Metadata Block System

Each block follows the structure: `[ID (1 byte)][Size (1 byte)][Data (Size bytes)]`.

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
| 3      | N    | DriverName | Null-terminated string.                                      |
| Var    | 2    | CodeTop    | Memory address where player code starts (typically `$1000`). |
| Var    | 2    | CodeSize   | Size of the executable code section.                         |
| Var    | 1    | Major      | Major version of the driver.                                 |
| Var    | 1    | Minor      | Minor version of the driver.                                 |
| Var    | 1    | Revision   | Revision version of the driver.                              |

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

When the `CodeTop` in the file differs from the intended `LoadAddress`, the following relocation must occur:

1. **Absolute Relocation**:
   Scan for 3-byte opcodes with absolute addressing modes (`$0D, $0E, $1D, $1E, $20, $2D, $2E, $40, $4D, $4E, $6D, $6E, $8D, $8E, $AD, $AE, $CD, $CE, $ED, $EE`).
   If the operand (bytes 1-2) falls within the code/data range, add `Delta = (NewLoadAddress - OriginalCodeTop)`.

2. **Zero-Page Relocation**:
   Scan for 2-byte opcodes with ZP addressing.
   Shift the operand by `ZP_Delta = (NewZPBase - OriginalZPBase)`.

### 6.2 Packing

When writing the file:

1. Concatenate Player Code + Table Data + Music Data + Patterns.
2. Update all pointers in Metadata Blocks to match the new packed addresses.
3. Calculate the file-relative offset: `Offset = AbsoluteAddress - LoadAddress + 2`.

---

## 7. Auxiliary Data (Editor Metadata)

Stored at the end of the file. Structured as a series of data chunks (Songs, Authors, Colors, etc.) used purely by the SIDFactory II UI and ignored by the C64 player code.
