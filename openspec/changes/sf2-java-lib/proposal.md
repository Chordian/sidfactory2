## Why

SID Factory 2 needs a reusable Java library for parsing, modeling, and re-serializing SF2 preset files. Currently there is no clean, manipulatable Java model of an SF2 file — clients that need to inspect or modify SF2 data have to work with raw binary or ad-hoc parsers. A dedicated library with a well-designed model enables binary-exact round-trip (parse → serialize yields identical bytes).

## What Changes

- Create a new standalone Java project `sf2-java-lib` with no external runtime dependencies beyond Java standard library
- Implement a mutable in-memory model of all SF2 data sections
- Implement a parser that reads an SF2 file into the model
- Implement a serializer that writes the model back to an SF2 file, producing binary-identical output when no model changes were made

## Capabilities

### New Capabilities
- `sf2-model`: Mutable in-memory model of all SF2 data sections, with layout-aware table access, unpacked order lists/sequences, and raw data preservation
- `sf2-parse`: Parse binary SF2 files into an in-memory model
- `sf2-serialize`: Serialize the in-memory model back to a binary SF2 file with binary-exact round-trip guarantee

### Modified Capabilities
*(none — new project)*

## Impact

- New Java project under `sf2-java-lib/` at repository root
- No changes to existing SID Factory 2 code
- Java 21+ required for builds
- Build system: Maven with Spotless auto-formatting
- Testing: Unit tests for parse/serialize round-trip with reference `.sf2` files
