package io.sf2.sidfactory2.model;

public class InstDataPointerDescription {
    public int tableId;
    public int instrumentDataPointerPosition;
    public int pointerAndValue;
    public int instrumentDataConditionalValuePosition;
    public int conditionValueAndValue;
    public int conditionEqualityValue;
    public int tableDataType;
    public int tableJumpMarkerValuePosition;
    public int tableJumpMarkerValue;
    public int tableJumpDestinationIndexPosition;

    public int getTableId() { return tableId; }
    public void setTableId(int v) { tableId = v; }
    public int getInstrumentDataPointerPosition() { return instrumentDataPointerPosition; }
    public void setInstrumentDataPointerPosition(int v) { instrumentDataPointerPosition = v; }
    public int getPointerAndValue() { return pointerAndValue; }
    public void setPointerAndValue(int v) { pointerAndValue = v; }
    public int getInstrumentDataConditionalValuePosition() { return instrumentDataConditionalValuePosition; }
    public void setInstrumentDataConditionalValuePosition(int v) { instrumentDataConditionalValuePosition = v; }
    public int getConditionValueAndValue() { return conditionValueAndValue; }
    public void setConditionValueAndValue(int v) { conditionValueAndValue = v; }
    public int getConditionEqualityValue() { return conditionEqualityValue; }
    public void setConditionEqualityValue(int v) { conditionEqualityValue = v; }
    public int getTableDataType() { return tableDataType; }
    public void setTableDataType(int v) { tableDataType = v; }
    public int getTableJumpMarkerValuePosition() { return tableJumpMarkerValuePosition; }
    public void setTableJumpMarkerValuePosition(int v) { tableJumpMarkerValuePosition = v; }
    public int getTableJumpMarkerValue() { return tableJumpMarkerValue; }
    public void setTableJumpMarkerValue(int v) { tableJumpMarkerValue = v; }
    public int getTableJumpDestinationIndexPosition() { return tableJumpDestinationIndexPosition; }
    public void setTableJumpDestinationIndexPosition(int v) { tableJumpDestinationIndexPosition = v; }
}
