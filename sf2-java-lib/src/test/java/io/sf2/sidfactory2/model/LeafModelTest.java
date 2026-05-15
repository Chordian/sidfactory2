package io.sf2.sidfactory2.model;

import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.*;

class LeafModelTest {
    @Test
    void descriptorVersionString() {
        var d = new Descriptor();
        d.setDriverVersionMajor(11);
        d.setDriverVersionMinor(5);
        d.setDriverVersionRevision(0);
        assertEquals("11.5.0", d.getVersionString());
    }

    @Test
    void descriptorMissingRevision() {
        var d = new Descriptor();
        d.setDriverVersionMajor(11);
        d.setDriverVersionMinor(5);
        assertEquals("11.5.0", d.getVersionString());
    }

    @Test
    void instrumentDescriptorHoldsLabels() {
        var desc = new InstrumentDescriptor();
        desc.getCellDescription().add("Wave");
        desc.getCellDescription().add("Pulse");
        assertEquals(2, desc.getCellDescription().size());
    }

    @Test
    void tableDefinitionMutable() {
        var td = new TableDefinition();
        td.setType(TableType.INSTRUMENTS);
        td.setId(1);
        td.setDataLayout(DataLayout.ROW_MAJOR);
        assertEquals(TableType.INSTRUMENTS, td.getType());
        assertEquals(1, td.getId());
        assertEquals(DataLayout.ROW_MAJOR, td.getDataLayout());
    }
}
