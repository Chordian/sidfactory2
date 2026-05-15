package io.sf2.sidfactory2.model;

public enum HeaderBlockId {
    DESCRIPTOR(1),
    DRIVER_COMMON(2),
    DRIVER_TABLES(3),
    INSTRUMENT_DESCRIPTOR(4),
    MUSIC_DATA(5),
    TABLE_COLOR_RULES(6),
    TABLE_INSDEL_RULES(7),
    TABLE_ACTION_RULES(8),
    INSTRUMENT_DATA_DESCRIPTOR(9),
    END(0xFF);

    private final int value;

    HeaderBlockId(int value) {
        this.value = value;
    }

    public int value() {
        return value;
    }

    public static HeaderBlockId fromValue(int value) {
        for (HeaderBlockId block : values()) {
            if (block.value == value) return block;
        }
        throw new IllegalArgumentException("Unknown block ID: 0x" + Integer.toHexString(value));
    }
}
