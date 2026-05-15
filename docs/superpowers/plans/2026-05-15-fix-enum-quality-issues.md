# Fix Enum Quality Issues Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Fix consistency, fragility, and ergonomics issues in model enums and update tests.

**Architecture:** Update static factory methods in enums to be more strict about input values and add a helper method to `TableProperty` for better readability.

**Tech Stack:** Java, JUnit 5

---

### Task 1: Update TableType

**Files:**
- Modify: `sf2-java-lib/src/main/java/io/sf2/sidfactory2/model/TableType.java`

- [ ] **Step 1: Update fromValue to throw IllegalArgumentException**

```java
    public static TableType fromValue(int value) {
        for (TableType type : values()) {
            if (type.value == value) return type;
        }
        throw new IllegalArgumentException("Unknown table type: 0x" + Integer.toHexString(value));
    }
```

- [ ] **Step 2: Run tests to verify failure (optional as we haven't updated tests yet)**

### Task 2: Update AuxiliaryChunkType

**Files:**
- Modify: `sf2-java-lib/src/main/java/io/sf2/sidfactory2/model/AuxiliaryChunkType.java`

- [ ] **Step 1: Update fromId to throw IllegalArgumentException**

```java
    public static AuxiliaryChunkType fromId(int id) {
        for (AuxiliaryChunkType type : values()) {
            if (type.typeId == id) return type;
        }
        throw new IllegalArgumentException("Unknown auxiliary chunk type: 0x" + Integer.toHexString(id));
    }
```

### Task 3: Update DataLayout

**Files:**
- Modify: `sf2-java-lib/src/main/java/io/sf2/sidfactory2/model/DataLayout.java`

- [ ] **Step 1: Update fromValue to throw IllegalArgumentException**

```java
    public static DataLayout fromValue(int value) {
        if (value == 0) return ROW_MAJOR;
        if (value == 1) return COLUMN_MAJOR;
        throw new IllegalArgumentException("Unknown data layout: " + value);
    }
```

### Task 4: Update TableProperty

**Files:**
- Modify: `sf2-java-lib/src/main/java/io/sf2/sidfactory2/model/TableProperty.java`

- [ ] **Step 1: Add isSet(int bits) method**

```java
    public boolean isSet(int bits) {
        return (bits & bit) != 0;
    }
```

### Task 5: Update EnumTest

**Files:**
- Modify: `sf2-java-lib/src/test/java/io/sf2/sidfactory2/model/EnumTest.java`

- [ ] **Step 1: Update tests to verify exceptions and new method**

```java
    @Test
    void tableTypes() {
        assertEquals(0x80, TableType.INSTRUMENTS.value());
        assertEquals(0x81, TableType.COMMANDS.value());
        assertThrows(IllegalArgumentException.class, () -> TableType.fromValue(0xFF));
    }

    @Test
    void dataLayout() {
        assertEquals(0, DataLayout.ROW_MAJOR.value());
        assertEquals(1, DataLayout.COLUMN_MAJOR.value());
        assertEquals(DataLayout.COLUMN_MAJOR, DataLayout.fromValue(1));
        assertThrows(IllegalArgumentException.class, () -> DataLayout.fromValue(2));
    }

    @Test
    void tablePropertyFlags() {
        int props = 0x03; // both bits set
        assertTrue(TableProperty.hasFlag(props, TableProperty.ENABLE_INSERT_DELETE));
        assertTrue(TableProperty.hasFlag(props, TableProperty.LAYOUT_ADD_VERTICALLY));
        assertFalse(TableProperty.hasFlag(props, TableProperty.INDEX_AS_CONTINUOUS_MEMORY));
        
        assertTrue(TableProperty.ENABLE_INSERT_DELETE.isSet(props));
        assertTrue(TableProperty.LAYOUT_ADD_VERTICALLY.isSet(props));
        assertFalse(TableProperty.INDEX_AS_CONTINUOUS_MEMORY.isSet(props));
    }

    @Test
    void auxiliaryChunkTypes() {
        assertEquals(3, AuxiliaryChunkType.PLAY_MARKERS.typeId());
        assertEquals(AuxiliaryChunkType.SONGS, AuxiliaryChunkType.fromId(5));
        assertThrows(IllegalArgumentException.class, () -> AuxiliaryChunkType.fromId(99));
    }
```

- [ ] **Step 2: Run all tests**

Run: `mvn test -Dtest=EnumTest`
Expected: PASS

- [ ] **Step 3: Commit all changes**

```bash
git add sf2-java-lib/src/main/java/io/sf2/sidfactory2/model/*.java sf2-java-lib/src/test/java/io/sf2/sidfactory2/model/EnumTest.java
git commit -m "refactor: fix quality issues in enums and update tests"
```
