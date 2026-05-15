## Context

SID Factory 2 (sf2) uses a binary format with embedded player code. Currently
there is no standalone Java model for reading, inspecting, or modifying the
data portion of an SF2 file. The player code must be preserved verbatim during
any round-trip or JSON-apply operation.

## Reference Sources

The implementation SHALL be derived from these authoritative sources:

### A. Format Specification

`doc/SF2_Speficifation.md` (v1.1) — Implementation-agnostic binary format spec covering:

- File topology: C64 PRG with `0x1337` signature, metadata TLV blocks, player
  code, song data, auxiliary data
- Metadata blocks 0x01–0x09: all field-level binary layouts for descriptor,
  driver common, tables, instrument descriptor, music data, color/insdel/action
  rules, inst data descriptor
- Song data formats: packed order list, packed sequence, row-major/column-major
  table layouts
- Jump/loop logic in instrument sub-tables
- Section 6: relocation and packing — **critical**: the Java library does NOT
  implement relocation/packing (that's the editor's job), but the model must be
  capable of round-trip without it
- Section 7: auxiliary data chunk format (5 chunk types)

### B. Editor C++ Reference Implementation

- `source/runtime/editor/driver/driver_info.h` + `driver_info.cpp` — `DriverInfo` class: reference parser with `ParseHeader()`, `ParseDescriptor()`, `ParseDriverCommon()`, `ParseDriverTables()`, `ParseMusicData()`, `ParseAuxilaryData()`. This is the canonical parse implementation the Java parser SHALL mirror.
- `source/utils/c64file.h` — `C64FileReader`/`C64FileWriter` utilities: sequential read/write of words, bytes, null-terminated strings. Java equivalents SHALL follow the same API pattern.
- `source/runtime/editor/utilities/editor_facility.cpp` — `SaveFile()` (line 756): reference serialize implementation that reads from emulated C64 memory, appends IRQ vector and auxiliary data, adjusts file header vectors.
- `source/runtime/editor/converters/utils/sf2_interface.h` + `sf2_interface.cpp` — `SF2::Interface` class: conversion API with read/write access to all SF2 data structures. Reference for how order lists, sequences, and tables are read/written in the data model.
- `source/runtime/editor/driver/driver_state.h` — Playback state model.
- `source/runtime/editor/auxilarydata/auxilary_data_collection.h` — Auxiliary data serialization: 5 chunk types with `Save()`/`Load()`.

### C. 6502 Driver Sources

`/Users/michel/dev/micheldebree/sidfactory2drivers/` — Kick Assembler driver implementations containing the canonical metadata block layout:

- `common/sf2_driver_defines.asm` — Central format constants: all `HEADER_BLOCK_ID_*` values, table type/ property/layout enums, color codes. These constants define the exact byte values the parser SHALL expect.
- Each driver (v11–v17, bonkers, np20, etc.) implements the metadata blocks in 6502 `.byte` directives — authoritative reference for block layout and field ordering.
- Version scheme: 3-part (`major.minor.revision`), stored in descriptor block. Bit 7 of major flags legacy NP20.

## Goals / Non-Goals

**Goals:**

- Java 21+, Maven build, Spotless auto-formatting on build
- Java library with minimal runtime dependencies (Jackson + stdlib)
- Binary-exact round-trip: parse → serialize with no model changes produces identical bytes
- Full in-memory model of all SF2 data sections, designed for ergonomic client manipulation
- JSON serialization of the model (excluding player code, including driver version)
- Apply JSON model to existing SF2 file, replacing data sections, preserving player code
- Driver version compatibility check for JSON-apply

**Non-Goals:**

- Audio rendering or playback
- Editing/transformation utilities — the model is a data carrier, clients build manipulation on top
- Performance optimization beyond reasonable baseline — clarity over micro-optimization
- Relocation and packing (Section 6 of the format spec) — these are editor operations, not model semantics
- Player code analysis or disassembly — player code is opaque bytes

## Module Architecture

The library SHALL be organized into four cleanly separated packages with no circular dependencies:

```
io.sf2.sidfactory2.model      — SF2 data model (POJOs/records, no I/O)
io.sf2.sidfactory2.parser     — Binary SF2 reader → model (depends on model only)
io.sf2.sidfactory2.serializer — Model → binary SF2 writer (depends on model only)
io.sf2.sidfactory2.json       — Model ↔ JSON (depends on model only)
```

- **model**: Pure data types. No file reading, no binary writing, no JSON. All classes are Java records or simple POJOs. This is the only module other modules depend on.
- **parser**: Reads binary SF2 bytes → produces model instances. Depends on `model` only.
- **serializer**: Takes model instances → writes binary SF2 bytes. Depends on `model` only.
- **json**: Jackson-based JSON converter. Model ↔ JSON string. Depends on `model` only.

No module depends on parser, serializer, or json. Clients wire them together.

## Build & Tooling

- **Java 21** — uses records, sealed classes, pattern matching, text blocks
- **Maven** — single-module `pom.xml`
- **Spotless** — `spotless:check` runs on every `mvn verify`, `spotless:apply` auto-formats. Palantir or Google Java format.
- **Jackson** (`jackson-databind` + `jackson-datatype-jdk8`) for JSON
- **JUnit 5** — all tests
- **Zero additional runtime dependencies** beyond Jackson

## Data Model Design

### Order lists and sequences as parsed domain objects

Order lists SHALL be `List<OrderListEntry>` where `OrderListEntry = (int transpose, int sequenceIndex)`. Sequences SHALL be `List<SequenceEvent>` where `SequenceEvent` models all packed fields (note, instrument, command, duration, tie flag, gate). The serializer reconstructs the packed byte stream from these domain objects, guaranteeing round-trip identity.

### Table data with flat byte[] and layout-aware accessors

Tables use a flat `byte[]` with `get(row, col)` / `set(row, col, byte)` methods that handle row-major vs column-major layout internally. Metadata (columnCount, rowCount, dataLayout) is bundled with the table data.

### Testing strategy

- **Reference `.sf2` files** bundled as test resources for round-trip parse/serialize tests
- **Synthetic builders** for edge case coverage (minimal SF2, empty sections, corrupt data)
- Round-trip tests assert byte-for-byte identity for reference files

## Error Handling

All parsing, serialization, and I/O errors use **checked exceptions**:

- `Sf2ParseException extends Exception` — malformed or truncated input
- `Sf2VersionMismatchException extends Exception` — driver version mismatch on JSON-apply
- `Sf2Exception extends Exception` — base class for all library exceptions

Checked exceptions are idiomatic for I/O and parsing in Java. The try/catch burden is minimal because the API surface has few operations.

## Decisions

1. **Parser reads all sections into typed Java records/classes**
   - Rationale: Records provide immutability where appropriate, clear structure, and good JSON serialization defaults. Sections map 1:1 to block types from the format spec (0x01–0x09) and the `DriverInfo` C++ parser.
   - Reference: `DriverInfo::ParseHeader()` dispatch in `driver_info.h:233` — the Java model SHALL have one class per block type matching the C++ `Descriptor`, `DriverCommon`, `TableDefinition`, `MusicData`, `InstrumentDescriptor`, `InstrumentDataPointerDescription`, `TableColorRule`, `TableInsertDeleteRule`, `TableActionRule` structs.
   - Alternative considered: Raw byte buffers — rejected because the goal is an ergonomic model.

2. **Player code stored as opaque byte array, excluded from JSON**
   - Rationale: Player code is large, opaque binary data that doesn't belong in JSON. JSON-apply preserves it from the target SF2 file.
   - Reference: The format spec (Section 2) places player code between metadata terminator and song data. The editor treats player code as opaque bytes loaded into C64 memory.
   - Alternative considered: Including player code as base64 in JSON — rejected because JSON interchange is for data editing, not full SF2 transport.

3. **Driver version stored as metadata field in both binary model and JSON**
   - Rationale: JSON-apply must verify driver version matches. Storing it in JSON enables version-aware editing workflows.
   - Reference: `HEADER_BLOCK_ID_DESCRIPTOR` in `sf2_driver_defines.asm` and `Descriptor::m_DriverVersionMajor/Minor/Revision` in `driver_info.h:61-63`.

4. **Serializer mirrors parser structure exactly**
   - Rationale: Binary-exact round-trip is guaranteed by serializing sections in the same order and with the same padding/alignment as the parser expects.
   - Reference: The format spec's block ordering (Section 3), the C++ `C64FileWriter` API in `c64file.h`, and `SaveFile()` in `editor_facility.cpp:756`.
   - The serializer SHALL write blocks in the canonical order: load address → signature → metadata blocks (0x01 through 0x09 in order) → terminator 0xFF → player code → packed song data → auxiliary data.

5. **JSON-apply reads target SF2, validates driver version, replaces data sections, re-serializes**
   - Rationale: Simplest correct approach. No need to rebuild the file from scratch — reuse the parser/serializer with a patched model. The player code, driver metadata blocks, auxiliary data, and vectors (`$0FFB`, `$0FFD`) are preserved from the target.
   - Reference: The editor's `SaveFile()` preserves player code by keeping the original driver PRG and only writing music data.

6. **Model maps to the C64 memory layout**
   - Rationale: The SF2 format is fundamentally a C64 memory image. Table data is stored at absolute C64 addresses, order lists and sequences are packed byte streams. The model SHALL represent these as they exist in memory, not as logical abstractions.
   - Reference: The `MusicData` block (Section 3.6) stores absolute C64 addresses for order lists, sequences, and tables. The `SF2::Interface` in `sf2_interface.cpp` reads/writes data directly at C64 memory addresses via `PushAllDataToMemory()`.

7. **All auxiliary data included in model AND JSON**
   - Rationale: Song names, table text labels, play markers, hardware preferences, and editing preferences are all data users may want to inspect or edit via JSON. Including everything in JSON maximizes interchange utility. Player code remains the sole exclusion.
   - Reference: The 5 auxiliary chunk types in `auxilary_data_collection.h` and Section 7 of the format spec. The C++ `AuxilaryDataCollection::Save()`/`Load()` serialize these as a chunk sequence.

## Risks / Trade-offs

- [Binary drift] → If the SF2 format gains new sections, the library needs updates. Mitigation: clear error messages for unknown section IDs.
- [Player code size] → Large byte arrays in memory. Mitigation: acceptable for typical SF2 file sizes; stream to disk if needed later.
- [JSON schema coupling] → JSON model tied to Java model structure. Mitigation: document the JSON schema explicitly.
- [C64 address semantics] → The model uses C64 memory addresses for table pointers, order list pointers, etc. External consumers must understand the C64 address space. Mitigation: document the memory-addressed model clearly in the API.
- [Auxiliary data versioning] → Auxiliary chunk versions may change. Mitigation: the parser SHALL read any version but only write the version it read (preserve, don't upgrade).
