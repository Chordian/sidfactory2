## ADDED Requirements

### Requirement: Apply JSON to existing SF2
The system SHALL read an existing SF2 file, replace its data sections with data from a JSON model, and write a new SF2 file.

#### Scenario: Apply JSON produces valid SF2
- **WHEN** a valid JSON model is applied to an existing SF2 file
- **THEN** the output is a valid SF2 file with player code from the target SF2 and data from the JSON model

### Requirement: Player code preserved from target SF2
The system SHALL preserve the player code from the target SF2 file, not from the JSON model (which has no player code).

#### Scenario: Target player code in output
- **WHEN** a JSON model is applied to an SF2 file
- **THEN** the output SF2 contains the player code from the target SF2 file

### Requirement: Auxiliary data from JSON replaces target auxiliary data
The system SHALL use auxiliary data from the JSON model, replacing the auxiliary data in the target SF2 file.

#### Scenario: JSON auxiliary data in output
- **WHEN** a JSON model with auxiliary data is applied to an SF2 file
- **THEN** the output SF2 contains the auxiliary data from the JSON model, not the target

### Requirement: Driver version validation
The system SHALL verify that the driver version in the JSON model matches the driver version in the target SF2 file before applying.

#### Scenario: Matching driver versions succeeds
- **WHEN** the JSON model's driver version matches the target SF2's driver version
- **THEN** the apply operation succeeds

#### Scenario: Mismatched driver versions fails
- **WHEN** the JSON model's driver version does NOT match the target SF2's driver version
- **THEN** the apply operation fails with a descriptive error

### Requirement: JSON-apply round-trip identity
When a model is created by parsing an SF2, serialized to JSON, and applied back to the same SF2, the result SHALL be byte-identical to the original.

#### Scenario: Apply round-trip identical bytes
- **WHEN** an SF2 file is parsed, serialized to JSON, and applied back to the same SF2
- **THEN** the output SF2 bytes exactly match the original

### Requirement: Output to new file
The system SHALL write the result to a new file specified by the caller. The input SF2 file SHALL NOT be overwritten.

#### Scenario: Input file preserved
- **WHEN** a JSON model is applied to an SF2 file with output path specified
- **THEN** the input file remains unchanged and the output file is created at the specified path

### Requirement: Preserve missing sections from target
The system SHALL preserve sections from the target SF2 file that are absent from the JSON model.

#### Scenario: JSON has partial data
- **WHEN** the JSON model omits certain sections (e.g., auxiliary data)
- **THEN** those sections are copied from the target SF2 file to the output

### Requirement: Handle data section size changes
The system SHALL handle changes in data section sizes by repacking all data sections contiguously during serialization.

#### Scenario: Data sections grow
- **WHEN** the JSON model contains more data than the target SF2
- **THEN** the serializer repacks all sections and updates all pointers accordingly
