package io.sf2.sidfactory2.model;

import java.util.ArrayList;
import java.util.List;

public class PlayMarkerLayer {
    private List<Integer> eventPositions = new ArrayList<>();

    public List<Integer> getEventPositions() { return eventPositions; }
    public void setEventPositions(List<Integer> v) { eventPositions = v; }
}
