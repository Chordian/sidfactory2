package io.sf2.sidfactory2.model;

import java.util.ArrayList;
import java.util.List;

public class TableInsDelRule {
    public int targetTableId;
    public int targetCellIndex;
    public int evaluationCellIndex;
    public int evaluationCellMask;
    public int evaluationCellConditionalValue;

    public int getTargetTableId() { return targetTableId; }
    public void setTargetTableId(int v) { targetTableId = v; }
    public int getTargetCellIndex() { return targetCellIndex; }
    public void setTargetCellIndex(int v) { targetCellIndex = v; }
    public int getEvaluationCellIndex() { return evaluationCellIndex; }
    public void setEvaluationCellIndex(int v) { evaluationCellIndex = v; }
    public int getEvaluationCellMask() { return evaluationCellMask; }
    public void setEvaluationCellMask(int v) { evaluationCellMask = v; }
    public int getEvaluationCellConditionalValue() { return evaluationCellConditionalValue; }
    public void setEvaluationCellConditionalValue(int v) { evaluationCellConditionalValue = v; }
}
