package io.sf2.sidfactory2;

import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.*;

class ExceptionTest {
    @Test
    void sf2ExceptionIsChecked() {
        assertThrows(Sf2ParseException.class, () -> {
            throw new Sf2ParseException("test");
        });
    }

    @Test
    void parseExceptionHasMessage() {
        var ex = new Sf2ParseException("bad file");
        assertEquals("bad file", ex.getMessage());
    }

    @Test
    void versionMismatchExceptionHasMessage() {
        var ex = new Sf2VersionMismatchException("version mismatch");
        assertEquals("version mismatch", ex.getMessage());
    }
}
