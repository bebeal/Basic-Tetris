#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

#include "stdint.h"
#include "libk.h"
#include "machine.h"

// First 16 VGA colors
enum Color : uint8_t{
    Black,
    Blue,
    Green,
    Cyan,
    Red,
    Magenta,
    Brown,
    LightGray,
    DarkGray,
    LightBlue,
    LightGreen,
    LightCyan,
    LightRed,
    LightMagenta,
    Yellow,
    White
};

extern uint8_t* VGAMEM;
extern uint8_t* double_buffer;
static constexpr uint32_t NUM_COLORS = 256;
static constexpr uint32_t WIDTH = 320;
static constexpr uint32_t HEIGHT = 200;

uint8_t offset(uint8_t x, uint8_t y); 
void plot(uint32_t x, uint32_t y, Color color, bool write_to_buffer);
void clear_screen(); 
void buffer_to_screen();
void draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, Color color, bool write_to_buffer);
void polygon(int num_vertices, int *vertices, Color color, bool write_to_buffer);

void write_string( int colour, const char *string );

#endif
