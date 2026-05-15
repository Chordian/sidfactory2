## ADDED Requirements

### Requirement: Serialize model to JSON
The system SHALL produce a JSON representation of the SF2 model.

#### Scenario: JSON includes all data sections
- **WHEN** a populated model is serialized to JSON
- **THEN** all data sections are represented in the JSON output

### Requirement: JSON excludes player code
The system SHALL NOT include player code bytes in the JSON output.

#### Scenario: Player code absent from JSON
- **WHEN** a model with player code is serialized to JSON
- **THEN** the JSON output contains no player code field or data

### Requirement: JSON includes driver version
The system SHALL include the driver version string in the JSON output.

#### Scenario: Driver version in JSON
- **WHEN** a model with driver version is serialized to JSON
- **THEN** the JSON output contains the driver version as a string field

### Requirement: JSON includes all auxiliary data
The system SHALL include all auxiliary data chunks (play markers, hardware preferences, editing preferences, table text, songs) in the JSON output.

#### Scenario: All auxiliary data in JSON
- **WHEN** a model with populated auxiliary data is serialized to JSON
- **THEN** the JSON output contains all 5 auxiliary chunk types

### Requirement: Deserialize JSON to model
The system SHALL reconstruct a model from its JSON representation.

#### Scenario: JSON round-trip preserves data
- **WHEN** a model is serialized to JSON and deserialized back
- **THEN** all data fields match the original (player code comparison is N/A)
