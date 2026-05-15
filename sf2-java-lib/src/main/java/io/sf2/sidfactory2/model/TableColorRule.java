package io.sf2.sidfactory2.model;

import java.util.ArrayList;
import java.util.List;

public class TableColorRule {
    public int evaluationCellIndex;
    public int evaluationCellMask;
    public int evaluationCellConditionalValue;
    public int backgroundColor;

    public int getEvaluationCellIndex() { return evaluationCellIndex; }
    public void setEvaluationCellIndex(int v) { evaluationCellIndex = v; }
    public int getEvaluationCellMask() { return evaluationCellMask; }
    public void setEvaluationCellMask(int v) { evaluationCellMask = v; }
    public int getEvaluationCellConditionalValue() { return evaluationCellConditionalValue; }
    public void setEvaluationCellConditionalValue(int v) { evaluationCellConditionalValue = v; }
    public int getBackgroundColor() { return backgroundColor; }
    public void setBackgroundColor(int v) { backgroundColor = v; }
}
