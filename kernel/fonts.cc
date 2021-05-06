#include "fonts.h"
#include "debug.h"
#include "graphics.h"
#include "heap.h"
#include "stdint.h"

uint16_t *unicode = nullptr;
uint8_t header_size;
uint16_t num_glyphs;
uint16_t bytes_per_glyph;

void psf_init() {
    Debug::printf("Initializing font bitmap\n");
    uint16_t glyph = 0;
    // cast the address to PSF header struct 
    PSF_font *font = (PSF_font *)&_binary_Uni1_VGA16_psf_start;
    header_size = 3;
    num_glyphs = font->mode & PSF1_MODE512 ? 512 : 256;
    bytes_per_glyph = font->height;
    // is there a unicode table? 
    if (0) {
        Debug::printf("is unicode\n");
        // get the offset of the table 
        unsigned char *s = ((unsigned char *)&_binary_Uni1_VGA16_psf_start + 3 + num_glyphs * bytes_per_glyph);
        // allocate memory for translation table 
        unicode = (uint16_t *)malloc(16 * num_glyphs); // maps unicode value : glyph index
        while ((uint32_t) s < (uint32_t)_binary_Uni1_VGA16_psf_end) {
            uint16_t uc = (uint16_t)(((unsigned char *)s)[0]);
            if (uc == 0xFF) {
                glyph++;
                s++;
                continue;
            } else if (uc & 128) {
                // UTF-8 to unicode 
                if ((uc & 32) == 0) {
                    uc = ((s[0] & 0x1F) << 6) + (s[1] & 0x3F);
                    s++;
                } else if ((uc & 16) == 0) {
                    uc = ((((s[0] & 0xF) << 6) + (s[1] & 0x3F)) << 6) + (s[2] & 0x3F);
                    s += 2;
                } else if ((uc & 8) == 0) {
                    uc = ((((((s[0] & 0x7) << 6) + (s[1] & 0x3F)) << 6) + (s[2] & 0x3F)) << 6) + (s[3] & 0x3F);
                    s += 3;
                } else
                    uc = 0;
            }
            // save translation 
            unicode[uc] = glyph;
            s++;
        }
    } else {
        unicode = nullptr;
    }
}


void put_char(uint16_t c, int cx, int cy, Color fg, Color bg) {
    PSF_font *font = (PSF_font *)&_binary_Uni1_VGA16_psf_start;
    // unicode translation 
    if (unicode != nullptr) {
        c = unicode[c];
    }
    // get the glyph for the character. If there's no glyph for a given character, we'll use c as a glyph index 
    unsigned char *glyph = (unsigned char *)&_binary_Uni1_VGA16_psf_start + 3 + (c > 0 && c < num_glyphs ? c : 0) * bytes_per_glyph;
    // map top left pixel of bitmap to pixel (x,y) coordinates 
    uint32_t x = cx * 8;
    uint32_t y = cy * font->height;
    int mask[8]={1,2,4,8,16,32,64,128};
    // finally display pixels according to the bitmap */
    for(uint32_t py = 0; py < font->height; py++) {
        for(uint32_t px = 0; px < 8; px++) {
            Color color = bg;
            if ((glyph[py]) & mask[px]) {
                color = fg;
            }
            plot(px + x, py + y, color, false);
        }
    }
}

void put_char(uint16_t c, int cx, int cy) {
    put_char(c, cx, cy, White, Black);
}
