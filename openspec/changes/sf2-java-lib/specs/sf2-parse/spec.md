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
