package io.sf2.sidfactory2.model;

import java.util.ArrayList;
import java.util.List;

public class Songs {
    private int songCount;
    private int selectedSong;
    private List<String> songNames = new ArrayList<>();

    public int getSongCount() { return songCount; }
    public void setSongCount(int v) { songCount = v; }
    public int getSelectedSong() { return selectedSong; }
    public void setSelectedSong(int v) { selectedSong = v; }
    public List<String> getSongNames() { return songNames; }
    public void setSongNames(List<String> v) { songNames = v; }
}
