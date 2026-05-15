package io.sf2.sidfactory2.model;

import java.util.ArrayList;
import java.util.List;

public class InstrumentDescriptor {
    private List<String> cellDescription = new ArrayList<>();

    public List<String> getCellDescription() { return cellDescription; }
    public void setCellDescription(List<String> v) { cellDescription = v; }
}
