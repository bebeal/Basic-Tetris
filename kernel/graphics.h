#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

#include "stdint.h"
#include "libk.h"
#include "machine.h"

// First 16 VGA colors
enum Color : uint8_t {
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

extern uint8_t *VGAMEM;
static constexpr uint32_t NUM_COLORS = 256;
static constexpr uint32_t WIDTH = 320;
static constexpr uint32_t HEIGHT = 200;

uint32_t offset(uint32_t x, uint32_t y); 
void plot(uint32_t x, uint32_t y, Color color, uint8_t *double_buffer );
void clear_screen(); 
void buffer_to_screen(uint8_t *double_buffer);
void buffer_to_screen_tetris(uint8_t *double_buffer);
void draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, Color color, uint8_t *double_buffer);
void polygon(int num_vertices, int *vertices, Color color, uint8_t *double_buffer);

void write_string_textmode(int colour, const char *string);

#endif
