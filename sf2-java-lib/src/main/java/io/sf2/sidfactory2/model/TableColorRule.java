package io.sf2.sidfactory2.model;

import java.util.ArrayList;
import java.util.List;

public class TableColorRule {
    private int evaluationCellIndex;
    private int evaluationCellMask;
    private int evaluationCellConditionalValue;
    private int backgroundColor;

    public int getEvaluationCellIndex() { return evaluationCellIndex; }
    public void setEvaluationCellIndex(int v) { evaluationCellIndex = v; }
    public int getEvaluationCellMask() { return evaluationCellMask; }
    public void setEvaluationCellMask(int v) { evaluationCellMask = v; }
    public int getEvaluationCellConditionalValue() { return evaluationCellConditionalValue; }
    public void setEvaluationCellConditionalValue(int v) { evaluationCellConditionalValue = v; }
    public int getBackgroundColor() { return backgroundColor; }
    public void setBackgroundColor(int v) { backgroundColor = v; }
}

class TableColorRules {
    private List<TableColorRule> rules = new ArrayList<>();

    public List<TableColorRule> getRules() { return rules; }
    public void setRules(List<TableColorRule> v) { rules = v; }
}
