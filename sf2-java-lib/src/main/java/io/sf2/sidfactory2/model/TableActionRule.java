package io.sf2.sidfactory2.model;

import java.util.ArrayList;
import java.util.List;

public class TableActionRule {
    private int applicableCell;
    private int targetTableId;
    private int targetIndexCell;
    private int targetIndexMask;
    private int evaluationCellIndex;
    private int evaluationCellMask;
    private int evaluationCellConditionalValue;

    public int getApplicableCell() { return applicableCell; }
    public void setApplicableCell(int v) { applicableCell = v; }
    public int getTargetTableId() { return targetTableId; }
    public void setTargetTableId(int v) { targetTableId = v; }
    public int getTargetIndexCell() { return targetIndexCell; }
    public void setTargetIndexCell(int v) { targetIndexCell = v; }
    public int getTargetIndexMask() { return targetIndexMask; }
    public void setTargetIndexMask(int v) { targetIndexMask = v; }
    public int getEvaluationCellIndex() { return evaluationCellIndex; }
    public void setEvaluationCellIndex(int v) { evaluationCellIndex = v; }
    public int getEvaluationCellMask() { return evaluationCellMask; }
    public void setEvaluationCellMask(int v) { evaluationCellMask = v; }
    public int getEvaluationCellConditionalValue() { return evaluationCellConditionalValue; }
    public void setEvaluationCellConditionalValue(int v) { evaluationCellConditionalValue = v; }
}

class TableActionRules {
    private List<TableActionRule> rules = new ArrayList<>();

    public List<TableActionRule> getRules() { return rules; }
    public void setRules(List<TableActionRule> v) { rules = v; }
}
