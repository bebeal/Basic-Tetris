#include "graphics.h"
#include "stdint.h"
#include "debug.h"
// reference: http://www.brackeen.com/vga/basics.html

uint8_t* VGA = (uint8_t*)0xA0000;
uint8_t* double_buffer = new uint8_t[320*200];

int abs(int x) {
    if (x < 0) return -x; else return x;
}

// x < 0 -> -1
// x == 0 -> 0
// x > 0 -> 1
int sgn(int x) {
    if (x < 0) return -1; else return x == 0 ? 0 : 1;
}

// maps (x,y) coordinates to linear memory 
uint32_t offset(uint32_t x, uint32_t y) {
    return x + (y << 8) + (y << 6);
}

// plots a pixel at coordinates (x, y)
// writes to buffer if write_to_buffer is true, otherwise directly to VGA memory
// Note: screen's origin (0,0) starts in the top left of the screen 
void plot(uint32_t x, uint32_t y, Color color, bool write_to_buffer) {
    if (write_to_buffer) {
        double_buffer[offset(x, y)] = color;
    } else {
        VGA[offset(x, y)] = color;
    }
}

void clear_screen() {
    Debug::printf("*** Clearing Screen\n");
    for(uint32_t i = 0; i < 320 * 200; i++) {
        *VGA = Black;
    }
}
                                                     
// draws a line using Bresenham's line-drawing algorithm, which uses   
// no multiplication or division.                                      
void draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, Color color, bool write_to_buffer) {
  int dx = x2 - x1;      /* the horizontal distance of the line */
  int dy = y2 - y1;      /* the vertical distance of the line */
  int dxabs = abs(dx);
  int dyabs = abs(dy);
  int sdx = sgn(dx);
  int sdy = sgn(dy);
  int x = dyabs >> 1;
  int y = dxabs >> 1;
  int px = x1;
  int py = y1;
  int i;

  plot(px, py, color,  write_to_buffer);

  /* the line is more horizontal than vertical */
  if (dxabs >= dyabs) {
    for (i = 0; i < dxabs; i++) {
      y += dyabs;
      if (y >= dxabs) {
        y -= dxabs;
        py += sdy;
      }
      px += sdx;
      plot(px, py, color, write_to_buffer);
    }
  }
  /* the line is more vertical than horizontal */
  else {
    for (i=0;i<dyabs;i++) {
      x += dxabs;
      if (x >= dyabs) {
        x-=dyabs;
        px += sdx;
      }
      py += sdy;
      plot(px, py, color, write_to_buffer);
    }
  }
}

// draw a polygon given an array of vertices 
void polygon(int num_vertices, int *vertices, Color color, bool write_to_buffer) {
  int i;
  for(i = 0; i < num_vertices - 1; i++) {
    draw_line(vertices[(i << 1) + 0], vertices[(i << 1) + 1], vertices[(i << 1) + 2], vertices[(i << 1) + 3], color, write_to_buffer);
  }
  draw_line(vertices[0], vertices[1], vertices[(num_vertices << 1) - 2], vertices[(num_vertices << 1) - 1], color, write_to_buffer);
}

// need a shape class for tetris shapes
    // store location of "origin" of shape on screen, using top left as origin just as screen does
    // functions to draw diff tetris shapes at diff locations on screen
    // functions to shift tetris shapes left, down, right without having to redraw the entire shape


// For use in text mode
void write_string( int colour, const char *string ) {
    volatile char *video = (volatile char*)0xB8000;
    while( *string != 0 ) {
        *video++ = *string++;
        *video++ = colour;
    }
}