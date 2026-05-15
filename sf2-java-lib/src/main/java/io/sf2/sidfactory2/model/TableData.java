package io.sf2.sidfactory2.model;

public class TableData {
    private byte[] data;
    private int columnCount;
    private int rowCount;
    private DataLayout layout;

    public TableData() {}

    public TableData(byte[] data, int columnCount, int rowCount, DataLayout layout) {
        this.data = data;
        this.columnCount = columnCount;
        this.rowCount = rowCount;
        this.layout = layout;
    }

    public byte[] getData() { return data; }
    public void setData(byte[] data) { this.data = data; }
    public int getColumnCount() { return columnCount; }
    public void setColumnCount(int v) { columnCount = v; }
    public int getRowCount() { return rowCount; }
    public void setRowCount(int v) { rowCount = v; }
    public DataLayout getLayout() { return layout; }
    public void setLayout(DataLayout v) { layout = v; }

    public byte get(int row, int col) {
        int offset = layout == DataLayout.ROW_MAJOR
            ? row * columnCount + col
            : col * rowCount + row;
        return data[offset];
    }

    public void set(int row, int col, byte value) {
        int offset = layout == DataLayout.ROW_MAJOR
            ? row * columnCount + col
            : col * rowCount + row;
        data[offset] = value;
    }
}
