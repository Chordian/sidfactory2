package io.sf2.sidfactory2.model;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class Sf2Model {
    private int loadAddress;
    private int signature;
    private Descriptor descriptor = new Descriptor();
    private DriverCommon driverCommon = new DriverCommon();
    private List<TableDefinition> tables = new ArrayList<>();
    private Map<Integer, TableData> tableData = new HashMap<>();
    private InstrumentDescriptor instrumentDescriptor = new InstrumentDescriptor();
    private MusicData musicData = new MusicData();
    private Map<Integer, List<OrderListEntry>> orderLists = new HashMap<>();
    private Map<Integer, List<SequenceEvent>> sequences = new HashMap<>();
    private List<TableColorRules> colorRules = new ArrayList<>();
    private List<TableInsDelRules> insDelRules = new ArrayList<>();
    private List<TableActionRules> actionRules = new ArrayList<>();
    private InstDataDescriptor instDataDescriptor = new InstDataDescriptor();
    private byte[] playerCode = new byte[0];
    private AuxiliaryData auxiliaryData = new AuxiliaryData();
    private Map<String, byte[]> rawData = new HashMap<>();

    public int getLoadAddress() { return loadAddress; }
    public void setLoadAddress(int v) { loadAddress = v; }
    public int getSignature() { return signature; }
    public void setSignature(int v) { signature = v; }

    public String getDriverVersion() {
        return descriptor.getVersionString();
    }

    public Descriptor getDescriptor() { return descriptor; }
    public void setDescriptor(Descriptor v) { descriptor = v; }
    public DriverCommon getDriverCommon() { return driverCommon; }
    public void setDriverCommon(DriverCommon v) { driverCommon = v; }
    public List<TableDefinition> getTables() { return tables; }
    public void setTables(List<TableDefinition> v) { tables = v; }
    public Map<Integer, TableData> getTableData() { return tableData; }
    public void setTableData(Map<Integer, TableData> v) { tableData = v; }
    public InstrumentDescriptor getInstrumentDescriptor() { return instrumentDescriptor; }
    public void setInstrumentDescriptor(InstrumentDescriptor v) { instrumentDescriptor = v; }
    public MusicData getMusicData() { return musicData; }
    public void setMusicData(MusicData v) { musicData = v; }
    public Map<Integer, List<OrderListEntry>> getOrderLists() { return orderLists; }
    public void setOrderLists(Map<Integer, List<OrderListEntry>> v) { orderLists = v; }
    public Map<Integer, List<SequenceEvent>> getSequences() { return sequences; }
    public void setSequences(Map<Integer, List<SequenceEvent>> v) { sequences = v; }
    public List<TableColorRules> getColorRules() { return colorRules; }
    public void setColorRules(List<TableColorRules> v) { colorRules = v; }
    public List<TableInsDelRules> getInsDelRules() { return insDelRules; }
    public void setInsDelRules(List<TableInsDelRules> v) { insDelRules = v; }
    public List<TableActionRules> getActionRules() { return actionRules; }
    public void setActionRules(List<TableActionRules> v) { actionRules = v; }
    public InstDataDescriptor getInstDataDescriptor() { return instDataDescriptor; }
    public void setInstDataDescriptor(InstDataDescriptor v) { instDataDescriptor = v; }
    public byte[] getPlayerCode() { return playerCode; }
    public void setPlayerCode(byte[] v) { playerCode = v; }
    public AuxiliaryData getAuxiliaryData() { return auxiliaryData; }
    public void setAuxiliaryData(AuxiliaryData v) { auxiliaryData = v; }
    public Map<String, byte[]> getRawData() { return rawData; }
    public void setRawData(Map<String, byte[]> v) { rawData = v; }
}
