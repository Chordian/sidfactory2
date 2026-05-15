package io.sf2.sidfactory2.model;

import java.util.ArrayList;
import java.util.List;

public class TableText {
    private List<TableTextBucket> buckets = new ArrayList<>();

    public List<TableTextBucket> getBuckets() { return buckets; }
    public void setBuckets(List<TableTextBucket> v) { buckets = v; }
}
