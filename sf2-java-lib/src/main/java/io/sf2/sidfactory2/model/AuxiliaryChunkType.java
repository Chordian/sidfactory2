package io.sf2.sidfactory2.model;

public enum AuxiliaryChunkType {
    UNDEFINED(0),
    EDITING_PREFERENCES(1),
    HARDWARE_PREFERENCES(2),
    PLAY_MARKERS(3),
    TABLE_TEXT(4),
    SONGS(5);

    private final int value;

    AuxiliaryChunkType(int value) {
        this.value = value;
    }

    public int value() {
        return value;
    }

    public static AuxiliaryChunkType fromValue(int value) {
        for (AuxiliaryChunkType type : values()) {
            if (type.value == value) return type;
        }
        throw new IllegalArgumentException("Unknown auxiliary chunk type: 0x" + Integer.toHexString(value));
    }
}
