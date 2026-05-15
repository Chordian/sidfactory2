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

### Requirement: JSON byte array encoding
The system SHALL encode `byte[]` fields as uppercase hex strings without prefix (e.g. "0A1B2C").

#### Scenario: Byte array encoded as hex
- **WHEN** a table data byte array [0x0A, 0x1B, 0x2C] is serialized to JSON
- **THEN** the JSON contains the string "0A1B2C"

### Requirement: JSON address encoding
The system SHALL encode C64 absolute addresses as hex strings with `$` prefix (e.g. "$1000").

#### Scenario: Address encoded with $ prefix
- **WHEN** an address value 0x1000 is serialized to JSON
- **THEN** the JSON contains the string "$1000"

### Requirement: JSON enum encoding
The system SHALL encode enums and bitfields as descriptive strings (e.g. "ROW_MAJOR", "COLUMN_MAJOR", "EnableInsertDelete").

#### Scenario: Data layout encoded as string
- **WHEN** a table with row-major layout is serialized to JSON
- **THEN** the JSON contains "ROW_MAJOR" for the layout field

### Requirement: JSON driver version
The system SHALL include driver version as a top-level `"driverVersion"` string field in JSON output.

#### Scenario: Driver version at top level
- **WHEN** a model with driver version "11.05.00" is serialized
- **THEN** the JSON contains `"driverVersion": "11.05.00"` at the top level
