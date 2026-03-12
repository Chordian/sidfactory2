## ADDED Requirements

### Requirement: Overlay hides when entering fullscreen
When the viewport enters fullscreen mode, the overlay SHALL be automatically hidden to prevent visual glitches.

#### Scenario: Enable fullscreen while overlay is active
- **WHEN** user toggles fullscreen while overlay is enabled
- **THEN** the overlay SHALL be hidden immediately
- **AND** the overlay enabled state SHALL be preserved for restoration

#### Scenario: Enable overlay while in fullscreen
- **WHEN** user enables overlay while in fullscreen mode
- **THEN** the overlay SHALL remain hidden until fullscreen is exited
- **AND** the overlay SHALL be shown when exiting fullscreen if it was enabled

### Requirement: Overlay state restores correctly on exit fullscreen
When the viewport exits fullscreen mode, the overlay SHALL return to its previous state.

#### Scenario: Exit fullscreen with overlay previously enabled
- **WHEN** user exits fullscreen while overlay was enabled before entering fullscreen
- **THEN** the overlay SHALL be shown again automatically
- **AND** the window size and position SHALL be restored to overlay-adjusted dimensions

#### Scenario: Exit fullscreen with overlay previously disabled
- **WHEN** user exits fullscreen while overlay was disabled
- **THEN** the overlay SHALL remain hidden

### Requirement: Window operations skip in fullscreen mode
The overlay control SHALL NOT attempt to manipulate window size or position while in fullscreen mode.

#### Scenario: Toggle overlay while in fullscreen
- **WHEN** user toggles overlay while in fullscreen mode
- **THEN** no window size or position changes SHALL occur
- **AND** the overlay state SHALL be tracked for restoration when exiting fullscreen
