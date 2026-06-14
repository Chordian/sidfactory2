## Context

The tracklist in the editor displays orderlist entries showing transpose and sequence numbers. Users need to see the packed size (in hex) of each sequence to optimize memory usage. The `DataSourceSequence` class already provides `GetPackedSize()` method. The track rendering happens in `ComponentTrack::Refresh()` where orderlist entries are drawn at sequence_index 0.

## Goals / Non-Goals

**Goals:**
- Display packed size (hex) below transpose/sequence number in tracklist
- Only show when sequence has multiple rows (height > 1)
- Use existing `DataSourceSequence::GetPackedSize()` method
- Config option `Visualizer.Tracklist.ShowPackedSize` (default: 0/off) to toggle display

**Non-Goals:**
- Modify packing logic or data structures
- Change orderlist data model

## Decisions

1. **Location**: Add packed size display in `ComponentTrack::Refresh()` after drawing orderlist entry (sequence_index == 0), on the next row (current_y + 1) in the orderlist column (m_Position.m_X)

2. **Condition**: Only draw if config `Visualizer.Tracklist.ShowPackedSize` == 1 AND sequence length > 1 (i.e., the sequence spans multiple rows, leaving space below the orderlist entry). The orderlist entry occupies only the first row of its column; sequence data is in the adjacent column.

3. **Format**: Print as variable-width uppercase hex (e.g., "A", "1A", "FF", "100") matching display state casing (`inDisplayState.IsHexUppercase()`). Use new helper `ToHexValueString(unsigned int, bool)` for variable-width output.

4. **Color**: Reuse existing dimmed color `SequenceInstrumentEmpty` (dimmed/gray) to distinguish derived read-only data from editable orderlist values.

5. **Data Access**: Get sequence from `m_DataSourceSequenceList[order_list_entry.m_SequenceIndex]` and call `GetPackedSize()`. Use cached `m_PackedSize` - no extra `Pack()` calls needed (packing happens for playback).

6. **Casing**: Match `inDisplayState.IsHexUppercase()` like other hex values in the track view.

7. **Helper**: Add overload `ComponentTrack::ToHexValueString(unsigned int, bool)` for variable-width hex formatting.

8. **Config**: Add `Visualizer.Tracklist.ShowPackedSize` config option (int, default 0). Read once during `ComponentTrack` construction and cache as member variable `m_ShowPackedSize` (bool). Check cached value in `ComponentTrack::Refresh()` instead of reading config every frame. Add to distributed ini files (config.ini, user.default.ini).

## Risks / Trade-offs

- [Risk] Sequence might be 1 row tall → Mitigation: Check `sequence->GetLength() > 1` before drawing
- [Risk] Visual clutter if many sequences → Mitigation: Only show when space permits (height > 1) AND config enabled
- [Risk] Packed size changes after edit → Mitigation: Refresh already triggers on sequence changes via `SequenceChangedEvent`
- [Risk] Packed size exceeds 255 → Mitigation: Variable-width hex handles any size (max 1024 bytes = 0x400 = 3 hex digits)