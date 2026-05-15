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
        assertEquals("11.05.00", d.getVersionString());
    }

    @Test
    void descriptorMissingRevision() {
        var d = new Descriptor();
        d.setDriverVersionMajor(11);
        d.setDriverVersionMinor(5);
        assertEquals("11.05.00", d.getVersionString());
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

    @Test
    void tableColorRulesMutable() {
        var rules = new TableColorRules();
        var rule = new TableColorRule();
        rule.setBackgroundColor(0x12);
        rules.getRules().add(rule);
        assertEquals(1, rules.getRules().size());
        assertEquals(0x12, rules.getRules().get(0).getBackgroundColor());
    }

    @Test
    void tableActionRulesMutable() {
        var rules = new TableActionRules();
        var rule = new TableActionRule();
        rule.setTargetTableId(5);
        rules.getRules().add(rule);
        assertEquals(1, rules.getRules().size());
        assertEquals(5, rules.getRules().get(0).getTargetTableId());
    }

    @Test
    void tableInsDelRulesMutable() {
        var rules = new TableInsDelRules();
        var rule = new TableInsDelRule();
        rule.setTargetTableId(7);
        rules.getRules().add(rule);
        assertEquals(1, rules.getRules().size());
        assertEquals(7, rules.getRules().get(0).getTargetTableId());
    }

    @Test
    void driverCommonMutable() {
        var dc = new DriverCommon();
        dc.setInitAddress(0x1000);
        assertEquals(0x1000, dc.getInitAddress());
        dc.setInitAddress(0x2000);
        assertEquals(0x2000, dc.getInitAddress());
    }

    @Test
    void instDataDescriptorMutable() {
        var desc = new InstDataDescriptor();
        var p = new InstDataPointerDescription();
        p.setTableId(3);
        desc.getPointerDescriptions().add(p);
        assertEquals(1, desc.getPointerDescriptions().size());
        assertEquals(3, desc.getPointerDescriptions().get(0).getTableId());
    }

    @Test
    void musicDataMutable() {
        var md = new MusicData();
        md.setTrackCount(3);
        assertEquals(3, md.getTrackCount());
        md.setTrackCount(4);
        assertEquals(4, md.getTrackCount());
    }

    @Test
    void tableTypeEnum() {
        assertEquals(0x80, TableType.INSTRUMENTS.value());
        assertEquals(TableType.COMMANDS, TableType.fromValue(0x81));
        assertThrows(IllegalArgumentException.class, () -> TableType.fromValue(0xFF));
    }

    @Test
    void auxiliaryChunkTypeEnum() {
        assertEquals(1, AuxiliaryChunkType.EDITING_PREFERENCES.value());
        assertEquals(AuxiliaryChunkType.SONGS, AuxiliaryChunkType.fromValue(5));
        assertThrows(IllegalArgumentException.class, () -> AuxiliaryChunkType.fromValue(99));
    }

    @Test
    void headerBlockIdEnum() {
        assertEquals(1, HeaderBlockId.DESCRIPTOR.value());
        assertEquals(HeaderBlockId.END, HeaderBlockId.fromValue(0xFF));
        assertThrows(IllegalArgumentException.class, () -> HeaderBlockId.fromValue(0xFE));
    }

    @Test
    void tablePropertyEnum() {
        assertTrue(TableProperty.ENABLE_INSERT_DELETE.isSet(0x01));
        assertFalse(TableProperty.ENABLE_INSERT_DELETE.isSet(0x02));
        assertTrue(TableProperty.hasFlag(0x05, TableProperty.INDEX_AS_CONTINUOUS_MEMORY));
        assertEquals(0x04, TableProperty.INDEX_AS_CONTINUOUS_MEMORY.bit());
    }

    @Test
    void dataLayoutEnum() {
        assertEquals(DataLayout.ROW_MAJOR, DataLayout.valueOf("ROW_MAJOR"));
        assertEquals(DataLayout.COLUMN_MAJOR, DataLayout.valueOf("COLUMN_MAJOR"));
    }
}
