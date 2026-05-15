# SF2 Specification Update Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Update and complete the SF2 (SIDFactory II) file format specification to be unambiguous and comprehensive for parser/serializer implementation.

**Architecture:** Systematic update of the `SF2_Speficifation.md` file based on findings from the C++ source code (`driver_info.cpp`, `driver_info.h`).

**Tech Stack:** Markdown documentation.

---

### Task 1: Basic Terminology and Intro Cleanup

**Files:**
- Modify: `doc/SF2_Speficifation.md`

- [ ] **Step 1: Replace "SoundFont 2" with "SIDFactory II"**
Replace occurrences of "SoundFont 2" and "soundfont" (case-insensitive) with "SIDFactory II". Update the title and introduction.

- [ ] **Step 2: Commit cleanup**
```bash
git add doc/SF2_Speficifation.md
git commit -m "docs: rename format to SIDFactory II and remove soundfont mentions"
```

### Task 2: Update Section 3 (Metadata Blocks) with Detailed Layouts

**Files:**
- Modify: `doc/SF2_Speficifation.md`

- [ ] **Step 1: Define detailed layout for Block 0x01 (Descriptor)**
Update with major/minor/revision and `DriverType`.

- [ ] **Step 2: Define detailed layout for Block 0x02 (DriverCommon)**
List all addresses (2 bytes each) as absolute C64 memory addresses.

- [ ] **Step 3: Define detailed layout for Block 0x03 (DriverTables)**
Document the `TableDefinition` structure, including `DataLayout` and `Properties` flags.

- [ ] **Step 4: Define detailed layout for Block 0x04 (InstrumentDescriptor)**
Document the sequence of null-terminated strings.

- [ ] **Step 5: Define detailed layout for Block 0x05 (MusicData)**
Document the specific offsets and sizes for track counts, pointers, and sizes.

- [ ] **Step 6: Define detailed layouts for Rules Blocks (0x06, 0x07, 0x08)**
Document the byte-level structure of Color, InsDel, and Action rules.

- [ ] **Step 7: Define detailed layout for Block 0x09 (InstDataDescriptor)**
Document the `InstrumentDataPointerDescription` structure.

- [ ] **Step 8: Commit metadata updates**
```bash
git add doc/SF2_Speficifation.md
git commit -m "docs: add detailed binary layouts for all metadata blocks"
```

### Task 3: Clarify Addressing and Relocation

**Files:**
- Modify: `doc/SF2_Speficifation.md`

- [ ] **Step 1: Explicitly state address types**
Clarify that all pointers in metadata blocks are absolute C64 memory addresses (Little-Endian).
Clarify that file-relative offsets are calculated as `AbsoluteAddress - LoadAddress + 2`.

- [ ] **Step 2: Update relocation logic**
Add detail on opcode scanning if missing or vague.

- [ ] **Step 3: Commit addressing clarifications**
```bash
git add doc/SF2_Speficifation.md
git commit -m "docs: clarify absolute addressing vs file offsets"
```

### Task 4: Document Jump Instructions and Table Logic

**Files:**
- Modify: `doc/SF2_Speficifation.md`

- [ ] **Step 1: Detailed documentation of "Jump" logic in tables**
Explain how `TableDataType`, `JumpMarkerValue`, and `JumpDestinationIndexPosition` from Block 0x09 work together to define table loops/jumps.

- [ ] **Step 2: Commit jump logic documentation**
```bash
git add doc/SF2_Speficifation.md
git commit -m "docs: detail table jump and loop logic"
```

### Task 5: Final Review and TODO Removal

**Files:**
- Modify: `doc/SF2_Speficifation.md`

- [ ] **Step 1: Remove all remaining TODOs**
Ensure no "TODO" strings remain in the document.

- [ ] **Step 2: Final proofread for ambiguity**
Check relative vs memory position mentions.

- [ ] **Step 3: Commit final version**
```bash
git add doc/SF2_Speficifation.md
git commit -m "docs: finalize SF2 specification and remove all TODOs"
```
