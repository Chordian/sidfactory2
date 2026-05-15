## 1. Project Setup

- [ ] 1.1 Create Maven project with `pom.xml` (Java 21, JUnit 5, Spotless, Jackson)
- [ ] 1.2 Configure Spotless with Palantir/Google format, enforce on `mvn verify`
- [ ] 1.3 Set up package structure: `model`, `parser`, `serializer`, `json`
- [ ] 1.4 Verify `mvn clean verify` passes with Spotless

## 2. SF2 Internal Model

- [ ] 2.1 Define typed model records/classes for all SF2 metadata blocks (0x01–0x09)
- [ ] 2.2 Define `OrderListEntry` and `SequenceEvent` domain records
- [ ] 2.3 Implement `Sf2Table` with flat `byte[]` and layout-aware `get(row,col)`/`set(row,col,byte)`
- [ ] 2.4 Store player code as opaque `byte[]` in model
- [ ] 2.5 Store driver version (major.minor.revision) as metadata field
- [ ] 2.6 Define model classes for all 5 auxiliary data chunk types
- [ ] 2.7 Define checked exception classes: `Sf2Exception`, `Sf2ParseException`, `Sf2VersionMismatchException`

## 3. Binary Parser

- [ ] 3.1 Implement SF2 file reader with section header detection
- [ ] 3.2 Parse all data sections into corresponding model objects
- [ ] 3.3 Extract player code bytes from player code section
- [ ] 3.4 Extract driver version string from metadata
- [ ] 3.5 Handle malformed/truncated files with descriptive parse exceptions

## 4. Binary Serializer

- [ ] 4.1 Implement section writer that mirrors parser structure and ordering
- [ ] 4.2 Serialize all data section model objects to binary output
- [ ] 4.3 Write player code bytes from model unchanged
- [ ] 4.4 Verify binary-exact round-trip: parse → serialize produces identical bytes

## 5. JSON I/O

- [ ] 5.1 Implement JSON serializer for model (all data sections + auxiliary data, excluding player code)
- [ ] 5.2 Include driver version string in JSON output
- [ ] 5.3 Implement JSON deserializer to reconstruct model from JSON
- [ ] 5.4 Verify JSON round-trip preserves all data fields including auxiliary data

## 6. JSON Apply

- [ ] 6.1 Parse target SF2 file to obtain its model (including player code and driver version)
- [ ] 6.2 Validate driver version match between JSON model and target SF2
- [ ] 6.3 Replace data sections in target model with data from JSON model
- [ ] 6.4 Preserve player code from target SF2 (not from JSON, which has none)
- [ ] 6.5 Re-serialize the patched model to binary SF2
- [ ] 6.6 Verify apply round-trip identity: parse → JSON → apply to same file → identical bytes

## 7. Error Handling & Edge Cases

- [ ] 7.1 Write unit tests for malformed file parsing errors
- [ ] 7.2 Write unit tests for driver version mismatch errors
- [ ] 7.3 Write tests for edge cases (empty sections, minimal SF2, max-size sections)
