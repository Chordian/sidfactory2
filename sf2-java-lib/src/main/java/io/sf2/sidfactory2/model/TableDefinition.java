package io.sf2.sidfactory2.model;

public class TableDefinition {
    private TableType type;
    private int id;
    private int textFieldSize;
    private String name;
    private DataLayout dataLayout;
    private boolean propertyEnabledInsertDelete;
    private boolean propertyLayoutVertically;
    private boolean propertyIndexAsContinuousMemory;
    private int insertDeleteRuleId;
    private int enterActionRuleId;
    private int colorRuleId;
    private int address;
    private int columnCount;
    private int rowCount;
    private int visibleRowCount;

    public TableType getType() { return type; }
    public void setType(TableType v) { type = v; }
    public int getId() { return id; }
    public void setId(int v) { id = v; }
    public int getTextFieldSize() { return textFieldSize; }
    public void setTextFieldSize(int v) { textFieldSize = v; }
    public String getName() { return name; }
    public void setName(String v) { name = v; }
    public DataLayout getDataLayout() { return dataLayout; }
    public void setDataLayout(DataLayout v) { dataLayout = v; }
    public boolean isPropertyEnabledInsertDelete() { return propertyEnabledInsertDelete; }
    public void setPropertyEnabledInsertDelete(boolean v) { propertyEnabledInsertDelete = v; }
    public boolean isPropertyLayoutVertically() { return propertyLayoutVertically; }
    public void setPropertyLayoutVertically(boolean v) { propertyLayoutVertically = v; }
    public boolean isPropertyIndexAsContinuousMemory() { return propertyIndexAsContinuousMemory; }
    public void setPropertyIndexAsContinuousMemory(boolean v) { propertyIndexAsContinuousMemory = v; }
    public int getInsertDeleteRuleId() { return insertDeleteRuleId; }
    public void setInsertDeleteRuleId(int v) { insertDeleteRuleId = v; }
    public int getEnterActionRuleId() { return enterActionRuleId; }
    public void setEnterActionRuleId(int v) { enterActionRuleId = v; }
    public int getColorRuleId() { return colorRuleId; }
    public void setColorRuleId(int v) { colorRuleId = v; }
    public int getAddress() { return address; }
    public void setAddress(int v) { address = v; }
    public int getColumnCount() { return columnCount; }
    public void setColumnCount(int v) { columnCount = v; }
    public int getRowCount() { return rowCount; }
    public void setRowCount(int v) { rowCount = v; }
    public int getVisibleRowCount() { return visibleRowCount; }
    public void setVisibleRowCount(int v) { visibleRowCount = v; }
}
