## 1. Implementation

- [x] 1.1 Add `ToHexValueString(unsigned int, bool)` overload in ComponentTrack for variable-width hex
- [x] 1.2 Modify `ComponentTrack::Refresh()` to display packed size below orderlist entry (at m_Position.m_X, current_y + 1)
- [x] 1.3 Add condition to only show packed size when sequence->GetLength() > 1
- [x] 1.4 Format packed size as variable-width hex matching display state casing (IsHexUppercase)
- [x] 1.5 Use dimmed color (SequenceInstrumentEmpty) for packed size display
- [x] 1.6 Use cached GetPackedSize() - no extra Pack() calls
- [x] 1.7 Add cached config member `m_ShowPackedSize` to ComponentTrack, read once in constructor from `Global::instance().GetConfig()`
- [x] 1.8 Modify `ComponentTrack::Refresh()` to check `m_ShowPackedSize` instead of reading config
- [x] 1.9 Add `Visualizer.Tracklist.ShowPackedSize = 0` to SIDFactoryII/config.ini
- [x] 1.10 Add `Visualizer.Tracklist.ShowPackedSize = 0` to dist/documentation/user.default.ini

## 2. Testing

- [x] 2.1 Verify packed size displays for multi-row sequences (length > 1)
- [x] 2.2 Verify packed size hidden for single-row sequences (length == 1)
- [x] 2.3 Verify packed size updates after sequence edit (SequenceChangedEvent triggers refresh)
- [x] 2.4 Verify hex format: variable-width, correct casing, values > 255 (e.g., 100, 256, 1024)
- [x] 2.5 Verify color is dimmed (distinct from orderlist values)
- [x] 2.6 Verify packed size hidden when config is 0 (default)
- [x] 2.7 Verify packed size shows when config is 1 and sequence length > 1