## 1. Viewport Changes

- [x] 1.1 Add `IsFullScreen()` method to Viewport class in viewport.h
- [x] 1.2 Implement `IsFullScreen()` method in viewport.cpp using SDL_GetWindowFlags

## 2. OverlayControl Changes

- [x] 2.1 Add `m_IsFullScreen` member variable to OverlayControl class in overlay_control.h
- [x] 2.2 Add `SetFullScreenState(bool)` method declaration to overlay_control.h
- [x] 2.3 Implement `SetFullScreenState()` in overlay_control.cpp
- [x] 2.4 Modify `Update()` to skip window manipulation when m_IsFullScreen is true
- [x] 2.5 Add logic to disable overlay when entering fullscreen (no restore on exit)
- [x] 2.6 Modify `SetOverlayEnabled()` or similar to check m_IsFullScreen and reject if fullscreen is active

## 3. EditorFacility Integration

- [x] 3.1 Modify `ApplyFullScreenSetting()` to call `m_OverlayControl->SetFullScreenState()` before/after fullscreen toggle

## 4. Testing

- [x] 4.1 Test: Enable overlay, then enable fullscreen - overlay should hide
- [x] 4.2 Test: Enable fullscreen, then try to enable overlay - overlay should remain disabled
- [x] 4.3 Test: Exit fullscreen - overlay remains disabled (no auto-re-enable)
- [x] 4.4 Test: Rapidly toggle fullscreen and overlay - no visual glitches
- [x] 4.5 Test: Toggle fullscreen with overlay disabled - no changes to overlay state

## 5. Fix for F12 overlay toggle issue

- [x] 5.1 Reset m_FlipOverlayState when changing fullscreen state in ApplyFullScreenSetting to prevent pending toggles from executing after exiting fullscreen
