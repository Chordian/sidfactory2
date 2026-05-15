package io.sf2.sidfactory2.model;

public enum TableType {
    GENERIC(0x00),
    INSTRUMENTS(0x80),
    COMMANDS(0x81);

    private final int value;

    TableType(int value) {
        this.value = value;
    }

    public int value() {
        return value;
    }

    public static TableType fromValue(int value) {
        for (TableType type : values()) {
            if (type.value == value) return type;
        }
        return GENERIC;
    }
}
