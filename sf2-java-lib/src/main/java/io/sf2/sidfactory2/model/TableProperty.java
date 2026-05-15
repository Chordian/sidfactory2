package io.sf2.sidfactory2.model;

public enum TableProperty {
    ENABLE_INSERT_DELETE(0x01),
    LAYOUT_ADD_VERTICALLY(0x02),
    INDEX_AS_CONTINUOUS_MEMORY(0x04);

    private final int bit;

    TableProperty(int bit) {
        this.bit = bit;
    }

    public int bit() {
        return bit;
    }

    public static boolean hasFlag(int properties, TableProperty flag) {
        return (properties & flag.bit) != 0;
    }
}
