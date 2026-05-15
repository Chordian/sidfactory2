package io.sf2.sidfactory2.model;

public class DriverCommon {
    private int initAddress;
    private int stopAddress;
    private int updateAddress;
    private int sidChannelOffsetAddress;
    private int driverStateAddress;
    private int tickCounterAddress;
    private int orderListIndexAddress;
    private int sequenceIndexAddress;
    private int sequenceInUseAddress;
    private int currentSequenceAddress;
    private int currentTransposeAddress;
    private int currentSequenceEventDurationAddress;
    private int nextInstrumentAddress;
    private int nextCommandAddress;
    private int nextNoteAddress;
    private int nextNoteIsTiedAddress;
    private int tempoCounterAddress;
    private int triggerSyncAddress;
    private int noteEventTriggerSyncValue;
    private int reservedByte;
    private int reservedWord;

    public int getInitAddress() { return initAddress; }
    public void setInitAddress(int initAddress) { this.initAddress = initAddress; }
    public int getStopAddress() { return stopAddress; }
    public void setStopAddress(int stopAddress) { this.stopAddress = stopAddress; }
    public int getUpdateAddress() { return updateAddress; }
    public void setUpdateAddress(int updateAddress) { this.updateAddress = updateAddress; }
    public int getSidChannelOffsetAddress() { return sidChannelOffsetAddress; }
    public void setSidChannelOffsetAddress(int sidChannelOffsetAddress) { this.sidChannelOffsetAddress = sidChannelOffsetAddress; }
    public int getDriverStateAddress() { return driverStateAddress; }
    public void setDriverStateAddress(int driverStateAddress) { this.driverStateAddress = driverStateAddress; }
    public int getTickCounterAddress() { return tickCounterAddress; }
    public void setTickCounterAddress(int tickCounterAddress) { this.tickCounterAddress = tickCounterAddress; }
    public int getOrderListIndexAddress() { return orderListIndexAddress; }
    public void setOrderListIndexAddress(int orderListIndexAddress) { this.orderListIndexAddress = orderListIndexAddress; }
    public int getSequenceIndexAddress() { return sequenceIndexAddress; }
    public void setSequenceIndexAddress(int sequenceIndexAddress) { this.sequenceIndexAddress = sequenceIndexAddress; }
    public int getSequenceInUseAddress() { return sequenceInUseAddress; }
    public void setSequenceInUseAddress(int sequenceInUseAddress) { this.sequenceInUseAddress = sequenceInUseAddress; }
    public int getCurrentSequenceAddress() { return currentSequenceAddress; }
    public void setCurrentSequenceAddress(int currentSequenceAddress) { this.currentSequenceAddress = currentSequenceAddress; }
    public int getCurrentTransposeAddress() { return currentTransposeAddress; }
    public void setCurrentTransposeAddress(int currentTransposeAddress) { this.currentTransposeAddress = currentTransposeAddress; }
    public int getCurrentSequenceEventDurationAddress() { return currentSequenceEventDurationAddress; }
    public void setCurrentSequenceEventDurationAddress(int currentSequenceEventDurationAddress) { this.currentSequenceEventDurationAddress = currentSequenceEventDurationAddress; }
    public int getNextInstrumentAddress() { return nextInstrumentAddress; }
    public void setNextInstrumentAddress(int nextInstrumentAddress) { this.nextInstrumentAddress = nextInstrumentAddress; }
    public int getNextCommandAddress() { return nextCommandAddress; }
    public void setNextCommandAddress(int nextCommandAddress) { this.nextCommandAddress = nextCommandAddress; }
    public int getNextNoteAddress() { return nextNoteAddress; }
    public void setNextNoteAddress(int nextNoteAddress) { this.nextNoteAddress = nextNoteAddress; }
    public int getNextNoteIsTiedAddress() { return nextNoteIsTiedAddress; }
    public void setNextNoteIsTiedAddress(int nextNoteIsTiedAddress) { this.nextNoteIsTiedAddress = nextNoteIsTiedAddress; }
    public int getTempoCounterAddress() { return tempoCounterAddress; }
    public void setTempoCounterAddress(int tempoCounterAddress) { this.tempoCounterAddress = tempoCounterAddress; }
    public int getTriggerSyncAddress() { return triggerSyncAddress; }
    public void setTriggerSyncAddress(int triggerSyncAddress) { this.triggerSyncAddress = triggerSyncAddress; }
    public int getNoteEventTriggerSyncValue() { return noteEventTriggerSyncValue; }
    public void setNoteEventTriggerSyncValue(int noteEventTriggerSyncValue) { this.noteEventTriggerSyncValue = noteEventTriggerSyncValue; }
    public int getReservedByte() { return reservedByte; }
    public void setReservedByte(int reservedByte) { this.reservedByte = reservedByte; }
    public int getReservedWord() { return reservedWord; }
    public void setReservedWord(int reservedWord) { this.reservedWord = reservedWord; }
}
