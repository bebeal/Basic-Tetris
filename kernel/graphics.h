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
uint8_t offset(uint8_t x, uint8_t y); 
void clear_screen(); 

void write_string( int colour, const char *string );

#endif
