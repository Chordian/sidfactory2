## ADDED Requirements

### Requirement: Mutable data model
The system SHALL provide a mutable in-memory model of all SF2 data sections.

#### Scenario: Model fields are mutable
- **WHEN** a model instance is created
- **THEN** all fields can be read and written via getters and setters

#### Scenario: Model supports optimization operations
- **WHEN** a tool modifies the model (rearranges, deletes, or adds entries)
- **THEN** the model reflects the changes immediately without requiring rebuild

### Requirement: Table data with layout-aware access
The system SHALL store table data as flat byte arrays with row/column accessors that handle row-major and column-major layouts.

#### Scenario: Row-major table access
- **WHEN** a table has `DataLayout = ROW_MAJOR`
- **THEN** `get(row, col)` returns the byte at offset `row * columnCount + col`

#### Scenario: Column-major table access
- **WHEN** a table has `DataLayout = COLUMN_MAJOR`
- **THEN** `get(row, col)` returns the byte at offset `col * rowCount + row`

#### Scenario: Table data is mutable
- **WHEN** `set(row, col, value)` is called
- **THEN** the underlying byte array is updated at the layout-correct offset

### Requirement: Order lists as unpacked domain objects
The system SHALL represent order lists as `List<OrderListEntry>` where each entry has `transpose` and `sequenceIndex` fields.

#### Scenario: Order list entry structure
- **WHEN** an order list entry is accessed
- **THEN** it provides `transpose` (int) and `sequenceIndex` (int) as separate fields

### Requirement: Sequences as unpacked domain objects
The system SHALL represent sequences as `List<SequenceEvent>` where each event models note, instrument, command, duration, tie, and gate fields.

#### Scenario: Sequence event structure
- **WHEN** a sequence event is accessed
- **THEN** it provides optional `note`, `instrument`, `command`, `duration` fields and boolean `tie`, `gate`, `noteOff` flags

### Requirement: Driver version as computed property
The system SHALL expose driver version as a string (e.g. "11.05.00") derived from the descriptor's major, minor, and revision fields.

#### Scenario: Driver version formatting
- **WHEN** the descriptor has major=11, minor=5, revision=0
- **THEN** `driverVersion` returns "11.05.00"

#### Scenario: Missing revision defaults to 0
- **WHEN** the descriptor has major=11, minor=5, no revision byte
- **THEN** `driverVersion` returns "11.05.00"

### Requirement: Player code as opaque byte array
The system SHALL store player code as a mutable `byte[]` field on the model.

#### Scenario: Player code is accessible
- **WHEN** a model is loaded from an SF2 file
- **THEN** `playerCode` contains the exact bytes from the player code section

### Requirement: Auxiliary data in model
The system SHALL store all 5 auxiliary data chunk types (PlayMarkers, HardwarePreferences, EditingPreferences, TableText, Songs) as typed fields.

#### Scenario: All auxiliary chunks accessible
- **WHEN** a model is loaded from an SF2 file with auxiliary data
- **THEN** each chunk type is available as a typed object on `auxiliaryData`

### Requirement: Raw data preservation for unknown sections
The system SHALL preserve unrecognized data sections in a `rawData` map keyed by section identifier.

#### Scenario: Unknown block preserved
- **WHEN** parsing encounters an unrecognized block ID
- **THEN** its raw bytes are stored in `rawData` for round-trip safety
