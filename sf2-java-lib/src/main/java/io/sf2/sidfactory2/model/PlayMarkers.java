package io.sf2.sidfactory2.model;

import java.util.ArrayList;
import java.util.List;

public class PlayMarkers {
    private List<PlayMarkerLayer> layers = new ArrayList<>();

    public List<PlayMarkerLayer> getLayers() { return layers; }
    public void setLayers(List<PlayMarkerLayer> v) { layers = v; }
}
