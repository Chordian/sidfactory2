## ADDED Requirements

### Requirement: Parse binary SF2 into model
The system SHALL read a binary SF2 file and produce a complete in-memory model of all data sections.

#### Scenario: Parse valid SF2 file
- **WHEN** a valid SF2 file is provided as input
- **THEN** all data sections are parsed into typed model objects

#### Scenario: Parse produces player code bytes
- **WHEN** a valid SF2 file is parsed
- **THEN** the player code section is available as an opaque byte array in the model

#### Scenario: Parse produces driver version
- **WHEN** a valid SF2 file is parsed
- **THEN** the driver version string is extracted and stored in the model

### Requirement: Error handling for invalid input
The system SHALL reject malformed or unsupported SF2 files with a descriptive exception.

#### Scenario: Parse truncated file
- **WHEN** a truncated or corrupt SF2 file is provided
- **THEN** a descriptive parse exception is thrown

### Requirement: Parse preserves all data verbatim
The system SHALL read all bytes from data sections without transformation.

#### Scenario: Parse preserves section bytes
- **WHEN** any data section contains specific byte patterns
- **THEN** the model stores those bytes identically

### Requirement: Parse file header
The system SHALL read the 2-byte load address and 2-byte signature (0x1337) from the file header. If the signature does not match, a descriptive parse exception SHALL be thrown.

#### Scenario: Valid signature accepted
- **WHEN** the file contains signature 0x1337 at offset 2
- **THEN** parsing proceeds normally

#### Scenario: Invalid signature rejected
- **WHEN** the file contains a signature other than 0x1337
- **THEN** a parse exception is thrown with a message indicating the invalid signature

### Requirement: Handle optional Revision byte
The system SHALL handle the optional Revision byte in the Descriptor block (0x01). If the block ends before the Revision byte, it SHALL default to 0.

#### Scenario: Revision byte present
- **WHEN** the Descriptor block contains the Revision byte
- **THEN** the revision value is read from the block

#### Scenario: Revision byte absent
- **WHEN** the Descriptor block ends before the Revision byte
- **THEN** revision defaults to 0

### Requirement: Reject unknown block IDs
The system SHALL throw a descriptive exception when encountering an unrecognized metadata block ID.

#### Scenario: Unknown block ID encountered
- **WHEN** parsing encounters a block ID not in the range 0x01–0x09 or 0xFF
- **THEN** a parse exception is thrown with the unknown block ID in the message

### Requirement: Parse auxiliary data
The system SHALL parse auxiliary data chunks (PlayMarkers, HardwarePreferences, EditingPreferences, TableText, Songs) into typed model objects.

#### Scenario: Auxiliary data parsed
- **WHEN** the file contains auxiliary data after the song data
- **THEN** all 5 chunk types are parsed into typed objects on the model

### Requirement: Unpack order lists
The system SHALL unpack the raw order list byte stream into `List<OrderListEntry>` during parsing.

#### Scenario: Order list unpacked
- **WHEN** an order list byte stream is parsed
- **THEN** it produces a list of `OrderListEntry` with `transpose` and `sequenceIndex` fields

### Requirement: Unpack sequences
The system SHALL unpack the raw sequence byte stream into `List<SequenceEvent>` during parsing.

#### Scenario: Sequence unpacked
- **WHEN** a sequence byte stream is parsed
- **THEN** it produces a list of `SequenceEvent` with note, instrument, command, duration, tie, gate, and noteOff fields
