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
        return value == 1 ? COLUMN_MAJOR : ROW_MAJOR;
    }
}
