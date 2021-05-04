#ifndef _TETRIS_H_
#define _TETRIS_H_

#include "debug.h"
#include "graphics.h"
#include "stdint.h"
#include "pit.h"

/*
Screen is 320 x 200
Make the tetris "tank" 152 pixels tall
make each sublock 4 x 4 pixels

I shape: 4 sublocks in a row i.e.
.
.
.
.

L shape:
.
.
..

Cube Shape:
..
..


*/

class Shape {
   protected:
    char type;            // 7 diff tetris shapes
    uint8_t orientation;  // 0, 1, 2, 3
    uint32_t num_blocks;
    uint32_t x;
    uint32_t y;
    Color color;
    bool draw_to_buffer;
    static constexpr uint32_t BLOCK_SIZE = 4;
    static constexpr uint32_t NUM_ORIENTATIONS = 4;

   public:
    Shape(char type, uint8_t orientation, uint32_t num_blocks, uint32_t x, uint32_t y, Color color) : type(type), orientation(orientation), num_blocks(num_blocks), x(x), y(x), color(color), draw_to_buffer(false) {}
    Shape(char type, uint8_t orientation, uint32_t num_blocks, uint32_t x, uint32_t y) : Shape(type, orientation, num_blocks, x, y, (Color) (Pit::jiffies % NUM_COLORS)) {}

    virtual void move_down() = 0;

    virtual void move_right() = 0;

    virtual void move_left() = 0;

    virtual void rotate() = 0;

    void draw_block(uint32_t x_start, uint32_t y_start) {
        for (uint32_t line = 0; line < BLOCK_SIZE; line++) {
            draw_line(x_start, y_start + line, x_start + 4, y_start + line, color, draw_to_buffer);
        }
    }

    void clear_block(uint32_t x_start, uint32_t y_start) {
        for (uint32_t line = 0; line < BLOCK_SIZE; line++) {
            draw_line(x_start, y_start + line, x_start + 4, y_start + line, Black, draw_to_buffer);
        }
    }
};

// I
class IShape : public Shape {
   public:

    IShape(uint32_t x, uint32_t y, Color color) : Shape('I', 0, 4, x, y, color) {
        Debug::printf("shape initialized at : %d, %d\n", x, y);
        uint32_t y_start = y;
        for (uint32_t block = 0; block < num_blocks; block++) {
            draw_block(x, y_start);
            y_start += BLOCK_SIZE;
        }
    }

    /*
     * Before: .  After: 
     *         .         .
     *         .         .
     *         .         .
     *                   .
     */
    void extend_bottom() {
        clear_block(x, y);
        draw_block(x, y + num_blocks * BLOCK_SIZE);
        y += BLOCK_SIZE;
    }

    /*
     * Before: ....
     * After:   ....
     */
    void extend_right() {
        clear_block(x, y);
        draw_block(x + num_blocks * BLOCK_SIZE, y);
        x += BLOCK_SIZE;
    }

    /*
     * Before: ....
     * After: ....
     */
    void extend_left() {
        clear_block(x + BLOCK_SIZE * 3, y);
        draw_block(x - BLOCK_SIZE, y);
        x -= BLOCK_SIZE;
    }

    /*
     * Before: .  After:   or Before: ....  After:
     *         .         .                         ....
     *         .         .
     *         .         .
     *                   .
     */
    void move_down() {
        Debug::printf("moving down current pos : %d, %d\n", x, y);
        if (orientation == 0 || orientation == 2) {
            extend_bottom();
        } else {
            for (uint32_t block = 0; block < num_blocks; block++) {
                Debug::printf("x, y, x + BLOCK_SIZE * num_blocks - 1, y: %d %d %d %d\n",x, y, x + BLOCK_SIZE * num_blocks - 1, y);
                Debug::printf("x, y + BLOCK_SIZE, x + BLOCK_SIZE * num_blocks - 1, y + BLOCK_SIZE: %d %d %d %d\n",x, y + BLOCK_SIZE, x + BLOCK_SIZE * num_blocks - 1, y + BLOCK_SIZE);
                draw_line(x, y, x + BLOCK_SIZE * num_blocks, y, Black, draw_to_buffer);
                draw_line(x, y + BLOCK_SIZE, x + BLOCK_SIZE * num_blocks, y + BLOCK_SIZE, color, draw_to_buffer);
                y++;
            }
        }
        Debug::printf("new pos : %d, %d\n", x, y);
    }

    /*
     * Before: .  After:  . or Before: ....  After:  ....
     *         .          .                        
     *         .          .
     *         .          .
     *                   
     */
    void move_right() {
        Debug::printf("moving right current pos : %d, %d\n", x, y);
        if (orientation == 0 || orientation == 2) {
            for (uint32_t block = 0; block < num_blocks; block++) {
                draw_line(x, y, x, y + BLOCK_SIZE * num_blocks - 1, Black, draw_to_buffer);
                draw_line(x + BLOCK_SIZE, y, x + BLOCK_SIZE, y + BLOCK_SIZE * num_blocks - 1, color, draw_to_buffer);
                x++;
            }
        } else {
            extend_right();
        }
        Debug::printf("new pos : %d, %d\n", x, y);
    }

    /*
     * Before: .  After:.  or Before: ....  After:....
     *         .        .                        
     *         .        .
     *         .        .
     *                   
     */
    void move_left() {
        Debug::printf("moving left current pos : %d, %d\n", x, y);
        if (orientation == 0 || orientation == 2) {
            x -= BLOCK_SIZE;
            uint32_t start_x = x;
            for (uint32_t block = 0; block < num_blocks; block++) {
                draw_line(start_x, y, start_x, y + BLOCK_SIZE * num_blocks - 1, color, draw_to_buffer);
                draw_line(start_x + BLOCK_SIZE, y, start_x + BLOCK_SIZE, y + BLOCK_SIZE * num_blocks - 1, Black, draw_to_buffer);
                start_x++;
            }
        } else {
            extend_left();
        }
        Debug::printf("new pos : %d, %d\n", x, y);
    }

    void rotate() {
        Debug::printf("rotating current pos, orientation : %d, %d %d\n", x, y, orientation);
        if (orientation == 0 || orientation == 2) {
            for(uint32_t block = 0; block < num_blocks; block++) {
                clear_block(x, y);
                y += BLOCK_SIZE;
            }
            x += 2 * BLOCK_SIZE;
            y -= 2 * BLOCK_SIZE;
            for(uint32_t block = 0; block < num_blocks; block++) {
                draw_block(x, y);
                x -= BLOCK_SIZE;
            }
            x += BLOCK_SIZE;
        } else {
            for(uint32_t block = 0; block < num_blocks; block++) {
                clear_block(x, y);
                x += BLOCK_SIZE;
            }
            x -= 3 * BLOCK_SIZE;
            y += BLOCK_SIZE;
            for(uint32_t block = 0; block < num_blocks; block++) {
                draw_block(x, y);
                y -= BLOCK_SIZE;
            }
            y += BLOCK_SIZE;
        }
        orientation = (orientation + 1) % NUM_ORIENTATIONS;
        Debug::printf("after rotate current pos, orientation : %d, %d %d\n", x, y, orientation);
    }
};

class Tetris {
};

#endif