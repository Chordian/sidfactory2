package io.sf2.sidfactory2.model;

public class EditingPreferences {
    private int notationMode;
    private int eventPosHighlightOffset;
    private int eventPosHighlightInterval;

    public int getNotationMode() { return notationMode; }
    public void setNotationMode(int v) { notationMode = v; }
    public int getEventPosHighlightOffset() { return eventPosHighlightOffset; }
    public void setEventPosHighlightOffset(int v) { eventPosHighlightOffset = v; }
    public int getEventPosHighlightInterval() { return eventPosHighlightInterval; }
    public void setEventPosHighlightInterval(int v) { eventPosHighlightInterval = v; }
}
