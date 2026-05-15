package io.sf2.sidfactory2.model;

public class DriverCommon {
    public int initAddress;
    public int stopAddress;
    public int updateAddress;
    public int sidChannelOffsetAddress;
    public int driverStateAddress;
    public int tickCounterAddress;
    public int orderListIndexAddress;
    public int sequenceIndexAddress;
    public int sequenceInUseAddress;
    public int currentSequenceAddress;
    public int currentTransposeAddress;
    public int currentSequenceEventDurationAddress;
    public int nextInstrumentAddress;
    public int nextCommandAddress;
    public int nextNoteAddress;
    public int nextNoteIsTiedAddress;
    public int tempoCounterAddress;
    public int triggerSyncAddress;
    public int noteEventTriggerSyncValue;
    public int reservedByte;
    public int reservedWord;

    public int getInitAddress() { return initAddress; }
    public void setInitAddress(int v) { initAddress = v; }
    public int getStopAddress() { return stopAddress; }
    public void setStopAddress(int v) { stopAddress = v; }
    public int getUpdateAddress() { return updateAddress; }
    public void setUpdateAddress(int v) { updateAddress = v; }
    public int getSidChannelOffsetAddress() { return sidChannelOffsetAddress; }
    public void setSidChannelOffsetAddress(int v) { sidChannelOffsetAddress = v; }
    public int getDriverStateAddress() { return driverStateAddress; }
    public void setDriverStateAddress(int v) { driverStateAddress = v; }
    public int getTickCounterAddress() { return tickCounterAddress; }
    public void setTickCounterAddress(int v) { tickCounterAddress = v; }
    public int getOrderListIndexAddress() { return orderListIndexAddress; }
    public void setOrderListIndexAddress(int v) { orderListIndexAddress = v; }
    public int getSequenceIndexAddress() { return sequenceIndexAddress; }
    public void setSequenceIndexAddress(int v) { sequenceIndexAddress = v; }
    public int getSequenceInUseAddress() { return sequenceInUseAddress; }
    public void setSequenceInUseAddress(int v) { sequenceInUseAddress = v; }
    public int getCurrentSequenceAddress() { return currentSequenceAddress; }
    public void setCurrentSequenceAddress(int v) { currentSequenceAddress = v; }
    public int getCurrentTransposeAddress() { return currentTransposeAddress; }
    public void setCurrentTransposeAddress(int v) { currentTransposeAddress = v; }
    public int getCurrentSequenceEventDurationAddress() { return currentSequenceEventDurationAddress; }
    public void setCurrentSequenceEventDurationAddress(int v) { currentSequenceEventDurationAddress = v; }
    public int getNextInstrumentAddress() { return nextInstrumentAddress; }
    public void setNextInstrumentAddress(int v) { nextInstrumentAddress = v; }
    public int getNextCommandAddress() { return nextCommandAddress; }
    public void setNextCommandAddress(int v) { nextCommandAddress = v; }
    public int getNextNoteAddress() { return nextNoteAddress; }
    public void setNextNoteAddress(int v) { nextNoteAddress = v; }
    public int getNextNoteIsTiedAddress() { return nextNoteIsTiedAddress; }
    public void setNextNoteIsTiedAddress(int v) { nextNoteIsTiedAddress = v; }
    public int getTempoCounterAddress() { return tempoCounterAddress; }
    public void setTempoCounterAddress(int v) { tempoCounterAddress = v; }
    public int getTriggerSyncAddress() { return triggerSyncAddress; }
    public void setTriggerSyncAddress(int v) { triggerSyncAddress = v; }
    public int getNoteEventTriggerSyncValue() { return noteEventTriggerSyncValue; }
    public void setNoteEventTriggerSyncValue(int v) { noteEventTriggerSyncValue = v; }
    public int getReservedByte() { return reservedByte; }
    public void setReservedByte(int v) { reservedByte = v; }
    public int getReservedWord() { return reservedWord; }
    public void setReservedWord(int v) { reservedWord = v; }
}
