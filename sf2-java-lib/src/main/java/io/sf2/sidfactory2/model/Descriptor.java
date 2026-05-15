package io.sf2.sidfactory2.model;

public class Descriptor {
    public int driverType;
    public int driverSize;
    public String driverName;
    public int driverCodeTop;
    public int driverCodeSize;
    public int driverVersionMajor;
    public int driverVersionMinor;
    public int driverVersionRevision;

    public int getDriverType() { return driverType; }
    public void setDriverType(int driverType) { this.driverType = driverType; }

    public int getDriverSize() { return driverSize; }
    public void setDriverSize(int driverSize) { this.driverSize = driverSize; }

    public String getDriverName() { return driverName; }
    public void setDriverName(String driverName) { this.driverName = driverName; }

    public int getDriverCodeTop() { return driverCodeTop; }
    public void setDriverCodeTop(int driverCodeTop) { this.driverCodeTop = driverCodeTop; }

    public int getDriverCodeSize() { return driverCodeSize; }
    public void setDriverCodeSize(int driverCodeSize) { this.driverCodeSize = driverCodeSize; }

    public int getDriverVersionMajor() { return driverVersionMajor; }
    public void setDriverVersionMajor(int driverVersionMajor) { this.driverVersionMajor = driverVersionMajor; }

    public int getDriverVersionMinor() { return driverVersionMinor; }
    public void setDriverVersionMinor(int driverVersionMinor) { this.driverVersionMinor = driverVersionMinor; }

    public int getDriverVersionRevision() { return driverVersionRevision; }
    public void setDriverVersionRevision(int driverVersionRevision) { this.driverVersionRevision = driverVersionRevision; }

    public String getVersionString() {
        return driverVersionMajor + "." + driverVersionMinor + "." + driverVersionRevision;
    }
}
