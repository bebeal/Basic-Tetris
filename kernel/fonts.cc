#include "fonts.h"
#include "debug.h"
#include "graphics.h"
#include "heap.h"
#include "stdint.h"

uint16_t *unicode = nullptr; 
uint8_t  header_size;
uint16_t num_glyphs;
uint16_t bytes_per_glyph;
uint16_t char_height;
uint16_t char_width;

void psf_init() {
    Debug::printf("Initializing font bitmap\n");
    // cast the address to PSF header struct 
    PSF_font *font = (PSF_font *)&_binary_Uni1_VGA16_psf_start;
    header_size = 4;
    num_glyphs = font->mode & PSF1_MODE512 ? 512 : 256;
    bytes_per_glyph = font->height;
    char_height = font->height;
    char_width = 8;
    Debug::printf("Number of Glyphs: %d\nNumber of bytes per glyph: %d\n", num_glyphs, bytes_per_glyph);
    if (0) {  // font->mode & PSF1_MODEHASTAB, not doing this without a map, too memory intensive to use array indices as unicode values, largest encoded value is 0xfffd thus would need 0xfffd * 8 
        // get the offset of the table 
        unsigned char *s = (((unsigned char *)&_binary_Uni1_VGA16_psf_start) + header_size + num_glyphs * bytes_per_glyph);
        // allocate memory for translation table 
        //unicode = new uint16_t[512*16]; // maps unicode value : glyph index
        uint16_t largest = 0;
        for(uint32_t gi = 0; gi < num_glyphs; gi++) {
            uint16_t stop_byte = 0; // stops when stop_byte == PSF1_SEPERATOR
            while(stop_byte != PSF1_SEPARATOR) {
                uint16_t val = 0;
                if (((((uint16_t) s[1]) << 8) + s[0]) == PSF1_STARTSEQ) {
                    s += 2;
                }
                val = (((uint16_t) s[1]) << 8) + s[0];
                if (val > largest) {
                    largest = val;
                }
                s += 2;
                stop_byte = (((uint16_t) s[1]) << 8) + s[0];
                //unicode[val] = gi;
            }
            s += 2;
        }
    } else {
        unicode = nullptr;
    }
}

unsigned char* offset(uint16_t index) {
    // get the glyph for the index. If there's no glyph for a given index, we'll use the first glyph 
    return (unsigned char *)&_binary_Uni1_VGA16_psf_start + header_size + (index > 0 && index < num_glyphs ? index : 0) * bytes_per_glyph;;
}

void put_char(uint16_t index, uint32_t cx, uint32_t cy, Color fg, Color bg, bool write_to_buffer) {
    // get the glyph for the index. If there's no glyph for a given index, we'll use the first glyph 
    unsigned char *glyph = offset(index);
    // map top left pixel of bitmap to pixel (x,y) coordinates 
    uint32_t x = cx * char_width;
    uint32_t y = cy * char_height;
    int mask[8] = {128, 64, 32, 16, 8, 4, 2, 1};
    // finally display pixels according to the bitmap */
    for(uint32_t py = 0; py < char_height; py++) {
        for(uint32_t px = 0; px < char_width; px++) {
            Color color = bg;
            if ((glyph[py]) & mask[px]) {
                color = fg;
            }
            plot(px + x, py + y, color, write_to_buffer);
        }
    }
}

void put_char(uint16_t index, uint32_t cx, uint32_t cy, Color fg, bool write_to_buffer) {
    // get the glyph for the index. If there's no glyph for a given index, we'll use the first glyph 
    unsigned char *glyph = offset(index);
    // map top left pixel of bitmap to pixel (x,y) coordinates 
    uint32_t x = cx * char_width;
    uint32_t y = cy * char_height;
    int mask[8] = {128, 64, 32, 16, 8, 4, 2, 1};
    // finally display pixels according to the bitmap */
    for(uint32_t py = 0; py < char_height; py++) {
        for(uint32_t px = 0; px < char_width; px++) {
            if ((glyph[py]) & mask[px]) {
                plot(px + x, py + y, fg, write_to_buffer);
            }
        }
    }
}


void put_char(uint16_t index, uint32_t cx, uint32_t cy, bool write_to_buffer) {
    put_char(index, cx, cy, White, Black, write_to_buffer);
}

void put_char(uint16_t index, uint32_t cx, uint32_t cy, Color fg, Color bg) {
    put_char(index, cx, cy, fg, bg, false);
}

void put_char(uint16_t index, uint32_t cx, uint32_t cy, Color fg) {
    put_char(index, cx, cy, fg, false);
}


void put_char(uint16_t index, uint32_t cx, uint32_t cy) {
    put_char(index, cx, cy, White, Black, false);
}

void ex() {
    put_char('H', 14, 4);
    put_char('E', 15, 4);
    put_char('L', 16, 4);
    put_char('L', 17, 4);
    put_char('O', 18, 4);
    put_char(' ', 19, 4);
    put_char('W', 20, 4);
    put_char('O', 21, 4);
    put_char('R', 22, 4);
    put_char('L', 23, 4);
    put_char('D', 24, 4);
}