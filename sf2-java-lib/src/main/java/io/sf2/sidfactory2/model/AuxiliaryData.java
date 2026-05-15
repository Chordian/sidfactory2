package io.sf2.sidfactory2.model;

public class AuxiliaryData {
    private EditingPreferences editingPreferences = new EditingPreferences();
    private HardwarePreferences hardwarePreferences = new HardwarePreferences();
    private PlayMarkers playMarkers = new PlayMarkers();
    private TableText tableText = new TableText();
    private Songs songs = new Songs();

    public EditingPreferences getEditingPreferences() { return editingPreferences; }
    public void setEditingPreferences(EditingPreferences v) { editingPreferences = v; }
    public HardwarePreferences getHardwarePreferences() { return hardwarePreferences; }
    public void setHardwarePreferences(HardwarePreferences v) { hardwarePreferences = v; }
    public PlayMarkers getPlayMarkers() { return playMarkers; }
    public void setPlayMarkers(PlayMarkers v) { playMarkers = v; }
    public TableText getTableText() { return tableText; }
    public void setTableText(TableText v) { tableText = v; }
    public Songs getSongs() { return songs; }
    public void setSongs(Songs v) { songs = v; }
}
