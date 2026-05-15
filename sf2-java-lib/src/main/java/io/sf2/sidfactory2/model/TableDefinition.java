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
    public void setType(TableType type) { this.type = type; }
    public int getId() { return id; }
    public void setId(int id) { this.id = id; }
    public int getTextFieldSize() { return textFieldSize; }
    public void setTextFieldSize(int textFieldSize) { this.textFieldSize = textFieldSize; }
    public String getName() { return name; }
    public void setName(String name) { this.name = name; }
    public DataLayout getDataLayout() { return dataLayout; }
    public void setDataLayout(DataLayout dataLayout) { this.dataLayout = dataLayout; }
    public boolean isPropertyEnabledInsertDelete() { return propertyEnabledInsertDelete; }
    public void setPropertyEnabledInsertDelete(boolean propertyEnabledInsertDelete) { this.propertyEnabledInsertDelete = propertyEnabledInsertDelete; }
    public boolean isPropertyLayoutVertically() { return propertyLayoutVertically; }
    public void setPropertyLayoutVertically(boolean propertyLayoutVertically) { this.propertyLayoutVertically = propertyLayoutVertically; }
    public boolean isPropertyIndexAsContinuousMemory() { return propertyIndexAsContinuousMemory; }
    public void setPropertyIndexAsContinuousMemory(boolean propertyIndexAsContinuousMemory) { this.propertyIndexAsContinuousMemory = propertyIndexAsContinuousMemory; }
    public int getInsertDeleteRuleId() { return insertDeleteRuleId; }
    public void setInsertDeleteRuleId(int insertDeleteRuleId) { this.insertDeleteRuleId = insertDeleteRuleId; }
    public int getEnterActionRuleId() { return enterActionRuleId; }
    public void setEnterActionRuleId(int enterActionRuleId) { this.enterActionRuleId = enterActionRuleId; }
    public int getColorRuleId() { return colorRuleId; }
    public void setColorRuleId(int colorRuleId) { this.colorRuleId = colorRuleId; }
    public int getAddress() { return address; }
    public void setAddress(int address) { this.address = address; }
    public int getColumnCount() { return columnCount; }
    public void setColumnCount(int columnCount) { this.columnCount = columnCount; }
    public int getRowCount() { return rowCount; }
    public void setRowCount(int rowCount) { this.rowCount = rowCount; }
    public int getVisibleRowCount() { return visibleRowCount; }
    public void setVisibleRowCount(int visibleRowCount) { this.visibleRowCount = visibleRowCount; }
}
