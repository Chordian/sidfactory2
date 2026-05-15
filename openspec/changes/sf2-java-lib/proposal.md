## Why

SID Factory 2 needs a reusable Java library for parsing, modeling, and re-serializing SF2 preset files. Currently there is no clean, manipulatable Java model of an SF2 file — clients that need to inspect or modify SF2 data have to work with raw binary or ad-hoc parsers. A dedicated library with a well-designed model enables binary-exact round-trip (parse → serialize yields identical bytes), JSON interchange for inspection/editing, and applying JSON edits back into existing SF2 files.

## What Changes

- Create a new standalone Java project `sf2-java-lib` with no external runtime dependencies beyond Java standard library
- Implement a parser that reads an SF2 file into an in-memory model
- Implement a serializer that writes the model back to an SF2 file, producing binary-identical output when no model changes were made
- Implement a JSON serializer/deserializer for the model (excluding player code and including driver version)
- Support applying a JSON model to an existing SF2 file — replacing the SF2 data while preserving the player code — with a driver version compatibility check

## Capabilities

### New Capabilities
- `sf2-parse`: Parse binary SF2 files into an in-memory model
- `sf2-serialize`: Serialize the in-memory model back to a binary SF2 file with binary-exact round-trip guarantee
- `sf2-json-io`: Serialize/deserialize the SF2 model to/from JSON, excluding player code, including driver version
- `sf2-json-apply`: Apply a JSON model to an existing SF2 file, replacing data while preserving player code and verifying driver version match

### Modified Capabilities
*(none — new project)*

## Impact

- New Java project under `sf2-java-lib/` at repository root
- No changes to existing SID Factory 2 code
- Java 17+ required for builds
- Build system: Gradle or Maven
- Testing: Unit tests for parse/serialize round-trip, JSON round-trip, JSON-apply
