## Why

The tracklist currently shows transpose/sequence numbers but lacks visibility into the packed size of each sequence. Users need to see the packed size (in hex) to optimize memory usage and understand sequence compression efficiency without opening each sequence individually.

The display should be opt-in via configuration to avoid visual clutter for users who don't need it.

## What Changes

- Add packed size display (hex) below transpose/sequence number column in tracklist
- Only show packed size when sequence row height allows (multi-row sequences)
- Add config option `Visualizer.Tracklist.ShowPackedSize` (default: 0/off) to toggle display
- No breaking changes to existing functionality

## Capabilities

### New Capabilities
- `tracklist-packed-size-display`: Display packed size in hex for each sequence in tracklist when space permits and config enabled

### Modified Capabilities
- None

## Impact

- Tracklist UI component (likely in sequence/tracklist view)
- Sequence data model to expose packed size calculation
- Config system: new option `Visualizer.Tracklist.ShowPackedSize`
- No API, dependency, or system changes