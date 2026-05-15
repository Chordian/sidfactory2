package io.sf2.sidfactory2.model;

import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.*;

class EnumTest {
    @Test
    void headerBlockIds() {
        assertEquals(1, HeaderBlockId.DESCRIPTOR.id());
        assertEquals(0xFF, HeaderBlockId.END.id());
        assertEquals(HeaderBlockId.MUSIC_DATA, HeaderBlockId.fromId(5));
    }

    @Test
    void tableTypes() {
        assertEquals(0x80, TableType.INSTRUMENTS.value());
        assertEquals(0x81, TableType.COMMANDS.value());
        assertThrows(IllegalArgumentException.class, () -> TableType.fromValue(0xFF));
    }

    @Test
    void dataLayout() {
        assertEquals(0, DataLayout.ROW_MAJOR.value());
        assertEquals(1, DataLayout.COLUMN_MAJOR.value());
        assertEquals(DataLayout.COLUMN_MAJOR, DataLayout.fromValue(1));
        assertThrows(IllegalArgumentException.class, () -> DataLayout.fromValue(2));
    }

    @Test
    void tablePropertyFlags() {
        int props = 0x03; // both bits set
        assertTrue(TableProperty.hasFlag(props, TableProperty.ENABLE_INSERT_DELETE));
        assertTrue(TableProperty.hasFlag(props, TableProperty.LAYOUT_ADD_VERTICALLY));
        assertFalse(TableProperty.hasFlag(props, TableProperty.INDEX_AS_CONTINUOUS_MEMORY));

        assertTrue(TableProperty.ENABLE_INSERT_DELETE.isSet(props));
        assertTrue(TableProperty.LAYOUT_ADD_VERTICALLY.isSet(props));
        assertFalse(TableProperty.INDEX_AS_CONTINUOUS_MEMORY.isSet(props));
    }

    @Test
    void auxiliaryChunkTypes() {
        assertEquals(3, AuxiliaryChunkType.PLAY_MARKERS.typeId());
        assertEquals(AuxiliaryChunkType.SONGS, AuxiliaryChunkType.fromId(5));
        assertThrows(IllegalArgumentException.class, () -> AuxiliaryChunkType.fromId(99));
    }
}
