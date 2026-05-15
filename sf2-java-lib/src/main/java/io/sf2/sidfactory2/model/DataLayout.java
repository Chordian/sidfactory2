package io.sf2.sidfactory2.model;

public enum DataLayout {
    ROW_MAJOR(0),
    COLUMN_MAJOR(1);

    private final int value;

    DataLayout(int value) {
        this.value = value;
    }

    public int value() {
        return value;
    }

    public static DataLayout fromValue(int value) {
        if (value == 0) return ROW_MAJOR;
        if (value == 1) return COLUMN_MAJOR;
        throw new IllegalArgumentException("Unknown data layout: " + value);
    }
}
