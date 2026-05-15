package io.sf2.sidfactory2.model;

import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.*;

class TableModelTest {
    @Test
    void tableDataRowMajor() {
        byte[] data = {0, 1, 2, 3, 4, 5};
        var td = new TableData(data, 2, 3, DataLayout.ROW_MAJOR);
        assertEquals(0, td.get(0, 0));
        assertEquals(1, td.get(0, 1));
        assertEquals(2, td.get(1, 0));
        assertEquals(5, td.get(2, 1));
    }

    @Test
    void tableDataColumnMajor() {
        byte[] data = {0, 1, 2, 3, 4, 5};
        var td = new TableData(data, 2, 3, DataLayout.COLUMN_MAJOR);
        assertEquals(0, td.get(0, 0));
        assertEquals(3, td.get(0, 1));
        assertEquals(1, td.get(1, 0));
        assertEquals(5, td.get(2, 1));
    }

    @Test
    void tableDataSetUpdatesArray() {
        byte[] data = {0, 0, 0, 0};
        var td = new TableData(data, 2, 2, DataLayout.ROW_MAJOR);
        td.set(1, 0, (byte) 0xFF);
        assertEquals((byte) 0xFF, td.get(1, 0));
        assertEquals((byte) 0xFF, data[2]);
    }

    @Test
    void orderListEntryFields() {
        var e = new OrderListEntry(12, 5);
        assertEquals(12, e.getTranspose());
        assertEquals(5, e.getSequenceIndex());
    }

    @Test
    void sequenceEventFields() {
        var e = new SequenceEvent();
        e.setNote(60);
        e.setInstrument(3);
        e.setDuration(8);
        e.setTie(true);
        assertEquals(60, e.getNote());
        assertEquals(3, e.getInstrument());
        assertEquals(8, e.getDuration());
        assertTrue(e.isTie());
        assertNull(e.getCommand());
    }

    @Test
    void sf2ModelDriverVersion() {
        var m = new Sf2Model();
        m.getDescriptor().setDriverVersionMajor(11);
        m.getDescriptor().setDriverVersionMinor(5);
        m.getDescriptor().setDriverVersionRevision(0);
        assertEquals("11.5.0", m.getDriverVersion());
    }

    @Test
    void sf2ModelMutableFields() {
        var m = new Sf2Model();
        m.setLoadAddress(0x1000);
        m.setSignature(0x1337);
        m.setPlayerCode(new byte[]{1, 2, 3});
        assertEquals(0x1000, m.getLoadAddress());
        assertEquals(0x1337, m.getSignature());
        assertArrayEquals(new byte[]{1, 2, 3}, m.getPlayerCode());
    }
}
