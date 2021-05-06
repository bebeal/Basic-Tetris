#include "fonts.h"
#include "debug.h"
#include "graphics.h"
#include "heap.h"
#include "stdint.h"

uint16_t *unicode = nullptr; 
uint8_t  header_size;
uint16_t num_glyphs;
uint16_t bytes_per_glyph;

void psf_init() {
    Debug::printf("Initializing font bitmap\n");
    // cast the address to PSF header struct 
    PSF_font *font = (PSF_font *)&_binary_Uni1_VGA16_psf_start;
    header_size = 4;
    num_glyphs = font->mode & PSF1_MODE512 ? 512 : 256;
    bytes_per_glyph = font->height;
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

void put_char(uint16_t index, int cx, int cy, Color fg, Color bg) {
    PSF_font *font = (PSF_font *)&_binary_Uni1_VGA16_psf_start;
    // get the glyph for the index. If there's no glyph for a given index, we'll use the first glyph 
    unsigned char *glyph = (unsigned char *)&_binary_Uni1_VGA16_psf_start + header_size + (index > 0 && index < num_glyphs ? index : 0) * bytes_per_glyph;
    // map top left pixel of bitmap to pixel (x,y) coordinates 
    uint32_t x = cx * 8;
    uint32_t y = cy * font->height;
    //int mask[8]={1,2,4,8,16,32,64,128};
    int mask[8] = {128, 64, 32, 16, 8, 4, 2, 1};
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

void put_char(uint16_t index, int cx, int cy) {
    put_char(index, cx, cy, White, Black);
}