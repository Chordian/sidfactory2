package io.sf2.sidfactory2.model;

public enum AuxiliaryChunkType {
    UNDEFINED(0),
    EDITING_PREFERENCES(1),
    HARDWARE_PREFERENCES(2),
    PLAY_MARKERS(3),
    TABLE_TEXT(4),
    SONGS(5);

    private final int typeId;

    AuxiliaryChunkType(int typeId) {
        this.typeId = typeId;
    }

    public int typeId() {
        return typeId;
    }

    public static AuxiliaryChunkType fromId(int id) {
        for (AuxiliaryChunkType type : values()) {
            if (type.typeId == id) return type;
        }
        throw new IllegalArgumentException("Unknown auxiliary chunk type: 0x" + Integer.toHexString(id));
    }
}
