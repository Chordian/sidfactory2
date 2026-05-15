package io.sf2.sidfactory2.model;

public class SequenceEvent {
    private Integer note;
    private Integer instrument;
    private Integer command;
    private Integer duration;
    private boolean tie;
    private boolean gate;
    private boolean noteOff;

    public Integer getNote() { return note; }
    public void setNote(Integer v) { note = v; }
    public Integer getInstrument() { return instrument; }
    public void setInstrument(Integer v) { instrument = v; }
    public Integer getCommand() { return command; }
    public void setCommand(Integer v) { command = v; }
    public Integer getDuration() { return duration; }
    public void setDuration(Integer v) { duration = v; }
    public boolean isTie() { return tie; }
    public void setTie(boolean v) { tie = v; }
    public boolean isGate() { return gate; }
    public void setGate(boolean v) { gate = v; }
    public boolean isNoteOff() { return noteOff; }
    public void setNoteOff(boolean v) { noteOff = v; }
}
