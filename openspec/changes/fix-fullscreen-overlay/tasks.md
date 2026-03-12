## 1. Viewport Changes

- [ ] 1.1 Add `IsFullScreen()` method to Viewport class in viewport.h
- [ ] 1.2 Implement `IsFullScreen()` method in viewport.cpp using SDL_GetWindowFlags

## 2. OverlayControl Changes

- [ ] 2.1 Add `m_IsFullScreen` member variable to OverlayControl class in overlay_control.h
- [ ] 2.2 Add `SetFullScreenState(bool)` method declaration to overlay_control.h
- [ ] 2.3 Implement `SetFullScreenState()` in overlay_control.cpp
- [ ] 2.4 Modify `Update()` to skip window manipulation when m_IsFullScreen is true
- [ ] 2.5 Add logic to disable overlay when entering fullscreen (no restore on exit)
- [ ] 2.6 Modify `SetOverlayEnabled()` or similar to check m_IsFullScreen and reject if fullscreen is active

## 3. EditorFacility Integration

- [ ] 3.1 Modify `ApplyFullScreenSetting()` to call `m_OverlayControl->SetFullScreenState()` before/after fullscreen toggle

## 4. Testing

- [ ] 4.1 Test: Enable overlay, then enable fullscreen - overlay should hide
- [ ] 4.2 Test: Enable fullscreen, then try to enable overlay - overlay should remain disabled
- [ ] 4.3 Test: Exit fullscreen - overlay remains disabled (no auto-re-enable)
- [ ] 4.4 Test: Rapidly toggle fullscreen and overlay - no visual glitches
- [ ] 4.5 Test: Toggle fullscreen with overlay disabled - no changes to overlay state
