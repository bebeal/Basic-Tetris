#ifndef _FONTS_H_
#define _FONTS_H_

#include "stdint.h"
#include "graphics.h"

#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04

#define PSF1_MODE512 0x01
#define PSF1_MODEHASTAB 0x02
#define PSF1_MODE512 0x01
#define PSF1_MAGIC_OK(x) ((x)[0] == PSF1_MAGIC0 && (x)[1] == PSF1_MAGIC1)
#define PSF1_SEPARATOR 0xFFFF
#define PSF1_STARTSEQ 0xFFFE


typedef struct {
    uint8_t magic[2]; // 0x36 and 0x04
    uint8_t mode; // how many character in file an dif there is unicode info     /* 0 : 256 characters, no unicode_data 1 : 512 characters, no unicode_data 2 : 256 characters, with unicode_data 3 : 512 characters, with unicode_data */                                                                  
    uint8_t height; 
    // width hard capped at 8
} PSF_font;

// from elf object file made from a .psf file
extern char _binary_Uni1_VGA16_psf_start;
extern char _binary_Uni1_VGA16_psf_end;
extern char _binary_Uni1_VGA16_psf_size;
 
void psf_init();
void put_char(uint16_t index, uint32_t cx, uint32_t cy, Color fg, Color bg);
void put_char(uint16_t index, uint32_t cx, uint32_t cy, Color fg);
void put_char(uint16_t index, uint32_t cx, uint32_t cy);
unsigned char* offset(uint16_t index);

void ex();


#endif


//typedef struct {
//     uint32_t magic;         /* magic bytes to identify PSF */
//     uint32_t version;       /* zero */
//     uint32_t headersize;    /* offset of bitmaps in file, 32 */
//     uint32_t flags;         /* 0 if there's no unicode table */
//     uint32_t numglyph;      /* number of glyphs */
//     uint32_t bytesperglyph; /* size of each glyph */
//     uint32_t height;        /* height in pixels */
//     uint32_t width;         /* width in pixels */
// } PSF_font2;