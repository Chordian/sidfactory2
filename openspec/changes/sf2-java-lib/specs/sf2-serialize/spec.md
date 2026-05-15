## ADDED Requirements

### Requirement: Serialize model to binary SF2
The system SHALL write the in-memory model back to a binary SF2 file.

#### Scenario: Serialize produces valid SF2
- **WHEN** a populated model is serialized
- **THEN** the output is a valid SF2 binary file

### Requirement: Binary-exact round-trip
The system SHALL produce byte-identical output when an SF2 file is parsed and immediately serialized without model modifications.

#### Scenario: Round-trip produces identical bytes
- **WHEN** any valid SF2 file is parsed and immediately serialized
- **THEN** the output bytes exactly match the input bytes

### Requirement: Serialize preserves player code
The system SHALL write the player code byte array from the model into the output file unchanged.

#### Scenario: Player code preserved verbatim
- **WHEN** a model with player code bytes is serialized
- **THEN** the player code section in the output file matches the model bytes exactly

### Requirement: Canonical block ordering
The system SHALL write metadata blocks in canonical order: 0x01 (Descriptor) through 0x09 (InstDataDescriptor), followed by 0xFF terminator.

#### Scenario: Blocks written in order
- **WHEN** a model is serialized
- **THEN** the output contains blocks 0x01 through 0x09 in ascending ID order, terminated by 0xFF

### Requirement: Pack order lists
The system SHALL pack `List<OrderListEntry>` into the raw order list byte stream format during serialization.

#### Scenario: Order list packed
- **WHEN** an order list with entries is serialized
- **THEN** the output contains the packed byte stream with transposition bytes, sequence indices, and end markers

### Requirement: Pack sequences
The system SHALL pack `List<SequenceEvent>` into the raw sequence byte stream format during serialization.

#### Scenario: Sequence packed
- **WHEN** a sequence with events is serialized
- **THEN** the output contains the packed byte stream with duration tokens, note values, instrument/command changes, and end marker

### Requirement: Handle missing or empty sections
The system SHALL handle missing or empty model sections gracefully during serialization.

#### Scenario: Empty auxiliary data
- **WHEN** the model has no auxiliary data
- **THEN** no auxiliary data is written and no error occurs

#### Scenario: Empty table data
- **WHEN** a table definition exists but has no data
- **THEN** an empty byte array is written for that table
