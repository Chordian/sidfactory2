## ADDED Requirements

### Requirement: Packed size displayed in tracklist
The system SHALL display the packed size (in hexadecimal) of each sequence in the tracklist, positioned below the transpose/sequence number in the orderlist column, when enabled by configuration.

#### Scenario: Config enables packed size display
- **WHEN** config `Visualizer.Tracklist.ShowPackedSize` is set to 1
- **THEN** packed size display is enabled for eligible sequences

#### Scenario: Config disables packed size display
- **WHEN** config `Visualizer.Tracklist.ShowPackedSize` is set to 0 (default)
- **THEN** packed size is never displayed regardless of sequence length

#### Scenario: Multi-row sequence shows packed size when enabled
- **WHEN** config `Visualizer.Tracklist.ShowPackedSize` is 1 AND a sequence in the orderlist has length greater than 1
- **THEN** the packed size in hex is displayed on the row immediately below the transpose/sequence number (orderlist column, current_y + 1)

#### Scenario: Single-row sequence does not show packed size
- **WHEN** a sequence in the orderlist has length equal to 1
- **THEN** no packed size is displayed (insufficient vertical space in orderlist column)

#### Scenario: Packed size updates when sequence changes
- **WHEN** a sequence is modified and its packed size changes
- **THEN** the displayed packed size updates on next refresh (triggered by SequenceChangedEvent)

#### Scenario: Packed size format is variable-width hex
- **WHEN** packed size is displayed
- **THEN** it is shown as variable-width uppercase hexadecimal (e.g., "A", "1A", "FF", "100") matching display state casing

#### Scenario: Packed size uses dimmed color
- **WHEN** packed size is displayed
- **THEN** it uses the dimmed color (SequenceInstrumentEmpty) to distinguish from editable orderlist values