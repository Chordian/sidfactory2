package io.sf2.sidfactory2.model;

import java.util.ArrayList;
import java.util.List;

public class InstDataDescriptor {
    public List<InstDataPointerDescription> pointerDescriptions = new ArrayList<>();

    public List<InstDataPointerDescription> getPointerDescriptions() { return pointerDescriptions; }
    public void setPointerDescriptions(List<InstDataPointerDescription> v) { pointerDescriptions = v; }
}
