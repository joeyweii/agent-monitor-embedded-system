#include "test_framework.h"
#include "font.h"
#include <vector>

/**
 * Tests the real font.cpp bitmap data.
 * font_5x7[c - 32] gives 5 column bytes for character c.
 * Each byte's bits [0..7] correspond to pixel rows [0..7].
 */

void test_font_ascii_range() {
    // The table covers ASCII 32 (' ') to 126 ('~') — 95 characters.
    // Spot-check that the table is populated (non-zero for non-space chars).
    
    // Space (index 0) should be all zeros
    for (int col = 0; col < 5; col++) {
        ASSERT_EQ(font_5x7[0][col], 0x00);
    }
    
    // '!' (index 1) should have at least one non-zero column
    bool excl_has_pixels = false;
    for (int col = 0; col < 5; col++) {
        if (font_5x7[1][col] != 0) excl_has_pixels = true;
    }
    ASSERT_TRUE(excl_has_pixels);
}

void test_font_glyph_A() {
    // 'A' is ASCII 65 -> index 65 - 32 = 33
    // Expected from font.cpp: {0x7E, 0x11, 0x11, 0x11, 0x7E}
    const uint8_t* glyph = font_5x7['A' - 32];
    ASSERT_EQ(glyph[0], 0x7E); // Left outer column:  01111110
    ASSERT_EQ(glyph[1], 0x11); // Second column:      00010001
    ASSERT_EQ(glyph[2], 0x11); // Center column:      00010001
    ASSERT_EQ(glyph[3], 0x11); // Fourth column:      00010001
    ASSERT_EQ(glyph[4], 0x7E); // Right outer column: 01111110
}

void test_font_glyph_zero() {
    // '0' is ASCII 48 -> index 48 - 32 = 16
    // Expected from font.cpp: {0x3E, 0x51, 0x49, 0x45, 0x3E}
    const uint8_t* glyph = font_5x7['0' - 32];
    ASSERT_EQ(glyph[0], 0x3E);
    ASSERT_EQ(glyph[1], 0x51);
    ASSERT_EQ(glyph[2], 0x49);
    ASSERT_EQ(glyph[3], 0x45);
    ASSERT_EQ(glyph[4], 0x3E);
}

void test_font_symmetry() {
    // Several characters like 'A', 'H', 'M', 'O', 'T', 'V', 'X'
    // have left-right symmetric bitmaps (col[0] == col[4], col[1] == col[3]).
    // This is a structural sanity check on the table data.
    
    const char symmetric_chars[] = "AHIMOTUVWX";
    for (const char* p = symmetric_chars; *p; p++) {
        const uint8_t* g = font_5x7[*p - 32];
        ASSERT_EQ(g[0], g[4]); // outer columns match
        ASSERT_EQ(g[1], g[3]); // inner columns match
    }
}

void register_font_tests(std::vector<TestCase>& tests) {
    tests.push_back({"font_ascii_range",  test_font_ascii_range});
    tests.push_back({"font_glyph_A",      test_font_glyph_A});
    tests.push_back({"font_glyph_zero",   test_font_glyph_zero});
    tests.push_back({"font_symmetry",     test_font_symmetry});
}
