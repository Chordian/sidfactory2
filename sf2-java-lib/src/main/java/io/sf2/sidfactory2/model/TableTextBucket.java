package io.sf2.sidfactory2.model;

import java.util.ArrayList;
import java.util.List;

public class TableTextBucket {
    private int tableId;
    private List<TableTextLayer> layers = new ArrayList<>();

    public int getTableId() { return tableId; }
    public void setTableId(int v) { tableId = v; }
    public List<TableTextLayer> getLayers() { return layers; }
    public void setLayers(List<TableTextLayer> v) { layers = v; }
}
