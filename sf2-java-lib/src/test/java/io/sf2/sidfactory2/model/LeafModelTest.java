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
        dc.initAddress = 0x2000;
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
        md.trackCount = 4;
        assertEquals(4, md.getTrackCount());
    }
}
