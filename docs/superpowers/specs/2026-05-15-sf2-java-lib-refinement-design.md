# SF2 Java Library Design — Model & Spec Refinement

**Date:** 2026-05-15
**Change:** sf2-java-lib
**Status:** Approved for implementation planning

## Summary

Refines the existing sf2-java-lib specs by:
1. Adding a new `sf2-model` capability — the mutable data model that all other capabilities depend on
2. Filling gaps in the existing 2 specs (parse, serialize)

Note: JSON capabilities (sf2-json-io, sf2-json-apply) deferred to a separate change.

## Model Design

### Mutable `Sf2Model` root

All fields are mutable (Java classes with getters/setters, not records).

```
Sf2Model
├── loadAddress (int)
├── signature (int, always 0x1337)
├── driverVersion (String, e.g. "11.05.00")
├── descriptor (Descriptor)
├── driverCommon (DriverCommon)
├── tables (List<TableDefinition>)
├── tableData (Map<tableId, TableData>)
├── instrumentDescriptor (InstrumentDescriptor)
├── musicData (MusicData)
├── orderLists (Map<trackIndex, List<OrderListEntry>>)
├── sequences (Map<seqIndex, List<SequenceEvent>>)
├── colorRules (List<TableColorRule>)
├── insDelRules (List<TableInsDelRule>)
├── actionRules (List<TableActionRule>)
├── instDataDescriptor (InstDataDescriptor)
├── playerCode (byte[])
├── auxiliaryData (AuxiliaryData)
└── rawData (Map<sectionKey, byte[]>)
```

### Key decisions

| Decision | Rationale |
|---|---|
| Mutable POJOs, not records | Optimization tools rearrange/delete entries in place |
| `tableData` uses `byte[]` with `get(row,col)`/`set(row,col)` | Layout (row-major vs column-major) handled internally |
| `orderLists`/`sequences` are unpacked domain objects | Ergonomic for manipulation; serializer packs on write |
| `rawData` preserves unrecognized sections | Forward-compatibility for unknown block IDs |
| `driverVersion` computed from descriptor | Single source of truth (major.minor.revision) |

### TableData

Each table stores a flat `byte[]` with metadata:

```java
class TableData {
    byte[] data;
    int columnCount;
    int rowCount;
    DataLayout layout; // ROW_MAJOR or COLUMN_MAJOR

    byte get(int row, int col);
    void set(int row, int col, byte value);
}
```

### OrderListEntry

```java
class OrderListEntry {
    int transpose;     // 0-127
    int sequenceIndex; // 0-255
}
```

### SequenceEvent

```java
class SequenceEvent {
    Integer note;          // 0-111, null = no note
    Integer instrument;    // 0-31, null = no change
    Integer command;       // 0-63, null = no change
    Integer duration;      // 0-15, null = same as previous
    boolean tie;           // tied to previous note
    boolean gate;          // gate on (sustain)
    boolean noteOff;       // gate off
}
```

## Gap Fixes for Existing Specs

### sf2-parse additions
- Parse file header (load address + 0x1337 signature validation)
- Handle optional Revision byte (default to 0 if absent)
- Reject unknown block IDs with descriptive error
- Parse auxiliary data chunks
- Unpack order lists: raw bytes → `List<OrderListEntry>`
- Unpack sequences: raw bytes → `List<SequenceEvent>`

### sf2-serialize additions
- Write blocks in canonical order: load address → signature → 0x01–0x09 → 0xFF → player code → song data → auxiliary data
- Pack `List<OrderListEntry>` → packed byte stream
- Pack `List<SequenceEvent>` → packed byte stream
- Handle missing/empty sections gracefully
