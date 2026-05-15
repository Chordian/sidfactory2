package io.sf2.sidfactory2.model;

public class OrderListEntry {
    private int transpose;
    private int sequenceIndex;

    public OrderListEntry() {}

    public OrderListEntry(int transpose, int sequenceIndex) {
        this.transpose = transpose;
        this.sequenceIndex = sequenceIndex;
    }

    public int getTranspose() { return transpose; }
    public void setTranspose(int v) { transpose = v; }
    public int getSequenceIndex() { return sequenceIndex; }
    public void setSequenceIndex(int v) { sequenceIndex = v; }
}
