## Why

The fullscreen and overlay features currently work independently without coordination. When users toggle fullscreen while an overlay is active (or activate an overlay while in fullscreen), visual glitches occur because the overlay control continues to manipulate window size and position as if in windowed mode, while fullscreen takes over the entire display. This causes inconsistent behavior and poor user experience.

## What Changes

- **Modified**: `OverlayControl` class to be fullscreen-aware
- **Modified**: `EditorFacility` to notify `OverlayControl` of fullscreen state changes
- **Modified**: `Viewport` to expose fullscreen state query
- **Modified**: Overlay rendering logic to adapt to fullscreen mode

### Specific Fixes

1. **Fullscreen state awareness**: `OverlayControl` will know when the viewport is in fullscreen mode and disable window size/position manipulation
2. **Fullscreen overlay rendering**: When in fullscreen, overlay images will scale to fill the screen or be hidden based on configuration
3. **State restoration**: Properly restore overlay window settings when exiting fullscreen
4. **Order-independent activation**: Both fullscreen and overlay can be toggled in any order without visual issues

## Capabilities

### New Capabilities
- `fullscreen-overlay`: Handling of overlay display in fullscreen mode (requires modification to existing overlay behavior)

### Modified Capabilities
- None - this is a bug fix that doesn't change the capability contract

## Impact

- **Files modified**:
  - `SIDFactoryII/source/runtime/editor/overlay_control.h` - Add fullscreen awareness
  - `SIDFactoryII/source/runtime/editor/overlay_control.cpp` - Implement fullscreen-aware overlay handling
  - `SIDFactoryII/source/runtime/editor/editor_facility.cpp` - Notify overlay of fullscreen changes
  - `SIDFactoryII/source/foundation/graphics/viewport.h` - Add fullscreen state query method
  - `SIDFactoryII/source/foundation/graphics/viewport.cpp` - Implement fullscreen state query
