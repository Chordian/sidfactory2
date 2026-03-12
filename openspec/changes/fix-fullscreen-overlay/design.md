## Context

The application has two independent display modes:

1. **Fullscreen mode**: Uses `SDL_WINDOW_FULLSCREEN_DESKTOP` to create a
   borderless fullscreen window at desktop resolution
2. **Overlay mode**: Displays PNG overlay images (bezels) around the editor,
   adjusting window size and position to accommodate the overlay

Currently, these modes don't communicate. The `OverlayControl` class
manipulates window size and position when enabling/disabling overlays, but
doesn't account for fullscreen state. When both are active simultaneously,
visual glitches occur because:

- Overlay tries to resize/move window in fullscreen mode (operations that have
  no effect or cause issues)
- Overlay rendering doesn't adapt to fullscreen dimensions
- Exiting fullscreen doesn't restore proper overlay window configuration

**Key files:**

- `overlay_control.cpp` - Manages overlay state, window size/position
- `viewport.cpp` - SDL window management, fullscreen toggle
- `editor_facility.cpp` - Coordinates between features

## Goals / Non-Goals

**Goals:**

- Make overlay behavior correct when entering/exiting fullscreen
- Allow toggling fullscreen and overlay in any order without visual issues
- Disable overlay in fullscreen mode
- Ensure smooth transitions between all state combinations

**Non-Goals:**

- Changing the fundamental overlay image format or loading mechanism
- Adding new overlay types beyond PNG
- Supporting multiple monitors or multi-window setups

## Decisions

### Decision 1: Fullscreen State Awareness

**Choice**: Add fullscreen state tracking to `Viewport` and pass it to `OverlayControl`

**Rationale**: The overlay control needs to know the fullscreen state to make
informed decisions. This keeps the fullscreen logic in the viewport (where SDL
calls happen) and allows overlay to query the state.

**Alternative considered**: Have `EditorFacility` manage both states and pass
explicit flags to both - rejected because it creates tighter coupling and
duplicates state management.

### Decision 2: Overlay Behavior in Fullscreen

**Choice**: Disable overlay when entering fullscreen, do not re-enable on exit.
User cannot enable overlay while fullscreen is active.

**Rationale**: Simplest implementation - just disable overlay when fullscreen
is enabled. Prevents user from accidentally enabling overlay in fullscreen mode
where it wouldn't display correctly.

**Alternative considered**: Scale overlay to fill - rejected because aspect
ratio would be wrong and it defeats the purpose of the bezel effect.

### Decision 3: State Synchronization

**Choice**: `EditorFacility::ApplyFullScreenSetting()` calls `OverlayControl`
to notify of fullscreen changes

**Rationale**: This is where fullscreen state changes, and it already has
access to both viewport and overlay control. Clean insertion point.

**Alternative considered**: Polling in overlay update - rejected as it creates
unnecessary coupling and timing issues.

## Risks / Trade-offs

1. **Timing issue with SDL fullscreen**: [Risk] SDL's fullscreen toggle may
   trigger window resize events that conflict with overlay adjustments →
   [Mitigation] Notify overlay of fullscreen state explicitly before/after SDL
   call, separate from window resize handling

2. **State restoration on crash**: [Risk] If app crashes in fullscreen, overlay
   state may be incorrect on next launch → [Mitigation] Store overlay enabled
   state in config, always initialize based on config not runtime state

3. **Race condition with rapid toggling**: [Risk] User rapidly toggles
   fullscreen/overlay could cause state inconsistency → [Mitigation] Add guard
   flag to prevent new state changes while transition is in progress

## Migration Plan

1. Add `IsFullScreen()` method to `Viewport` class
2. Add `SetFullScreenState(bool)` method to `OverlayControl`
3. Modify `EditorFacility::ApplyFullScreenSetting()` to call overlay notification
4. Modify `OverlayControl::Update()` to skip window manipulation when in fullscreen
5. Test all combinations: overlay→fullscreen, fullscreen→overlay, toggle both rapidly

No database or data migration needed. No rollback required as this is internal state handling.

## Open Questions

**Answered:**

1. **Overlay state preservation**: Reset to default - overlay will be disabled
   when entering fullscreen. No re-enabling on exit (simplest approach).

2. **User setting for overlay behavior**: No setting needed - simple disable
   behavior in fullscreen is sufficient.
