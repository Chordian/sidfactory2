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
