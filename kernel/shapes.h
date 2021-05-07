#ifndef _SHAPES_H_
#define _SHAPES_H_

#include "stdint.h"
#include "graphics.h"
#include "debug.h"
#include "pit.h"

//#define DEBUG
#ifdef DEBUG
  #define D if(1)
#else 
  #define D if(0)
#endif

enum class ShapeType {
    SQUARE,
    I,
    L,
    REVERSE_L,
    Z,
    REVERSE_Z,
    T
};

extern uint32_t to_x(uint32_t global_x, uint32_t block_num);
extern uint32_t to_y(uint32_t global_y, uint32_t block_num);
/*
extern static const uint32_t SQUARE_P[][];
extern static const uint32_t I_P[][];
extern static const uint32_t L_P[][];
extern static const uint32_t REV_L_P[][];
extern static const uint32_t Z_P[][];
extern static const uint32_t REV_Z_P[][];
extern static const uint32_t T_P[][];
*/


/*
General shape class all the tetris shapes inherent from, then forced to implement move_down(), move_right, move_left(), rotate
*/
class Shape {
protected:
    ShapeType type;       // 7 diff tetris shapes 
    uint8_t orientation;  // 0, 1, 2, 3
    uint32_t num_blocks;
    uint32_t on_blocks[4][4]; //  num_blocks length array of ints representing which 
                           // blocks in a 4x4 are "on" for the shape, for the current rotation
    uint32_t x; // top left pixel of 4x4, right is increasing x, x = column
    uint32_t y; // top left pixel of 4x4, down is increasing y, y = row
    Color color;
    
    //bool draw_to_buffer;
    static constexpr uint32_t BLOCK_SIZE = 4;
    static constexpr uint32_t NUM_ORIENTATIONS = 4;

    // change the orientation according to ShapeType
    void rotate_templ();

    // takes a relative block number (0-15) and translates to an absolute x val
    uint32_t to_x(uint32_t block_num);

    // takes a relative block number (0-15) and translates to absolute y val
    uint32_t to_y(uint32_t block_num);

    void clear_shape(uint8_t* double_buffer);
    void draw_shape(uint8_t* double_buffer);

   public:
    Shape(ShapeType type, uint8_t orientation, uint32_t num_blocks, uint32_t x, uint32_t y, Color color) : type(type), orientation(orientation), num_blocks(num_blocks), x(x), y(y), color(color) {}
    Shape(ShapeType type, uint8_t orientation, uint32_t num_blocks, uint32_t x, uint32_t y) : Shape(type, orientation, num_blocks, x, y, (Color) (Pit::jiffies % NUM_COLORS)) {}

    void move_down(uint8_t* double_buffer);
    void move_right(uint8_t* double_buffer);
    void move_left(uint8_t* double_buffer);
    void rotate(uint8_t* double_buffer);
    void fill_block(uint32_t x_start, uint32_t y_start, Color c, uint8_t* double_buffer);

    virtual ~Shape() {}
    friend class Tetris;


};

class SquareShape : public Shape {
    public:
    SquareShape(uint32_t x, uint32_t y, Color color);
    void rotate_templ();
};

class IShape : public Shape {
    public:
    IShape(uint32_t x, uint32_t y, Color color);
    void rotate_templ();
};

class LShape : public Shape {
    public:
    LShape(uint32_t x, uint32_t y, Color color);
    void rotate_templ();
};

class ReverseLShape : public Shape {
    public:
    ReverseLShape(uint32_t x, uint32_t y, Color color);
    void rotate_templ();
};

class ZShape : public Shape {
    public:
    ZShape(uint32_t x, uint32_t y, Color color);
    void rotate_templ();
};

class ReverseZShape : public Shape {
    public:
    ReverseZShape(uint32_t x, uint32_t y, Color color);
    void rotate_templ();
};

class TShape : public Shape {
    public:
    TShape(uint32_t x, uint32_t y, Color color);
    void rotate_templ();
};

/*
class IShape : public Shape {
   public:

    IShape(uint32_t x, uint32_t y, Color color) : Shape(ShapeType::I, 0, 4, x, y, color) {
        D Debug::printf("Ishape initialized at : %d, %d\n", x, y);
        uint32_t y_start = y;
        for (uint32_t block = 0; block < num_blocks; block++) {
            draw_block(x, y_start);
            y_start += BLOCK_SIZE;
        }
    }

    void extend_bottom() {
        clear_block(x, y);
        draw_block(x, y + num_blocks * BLOCK_SIZE);
        y += BLOCK_SIZE;
    }

    void extend_right() {
        clear_block(x, y);
        draw_block(x + num_blocks * BLOCK_SIZE, y);
        x += BLOCK_SIZE;
    }

    void extend_left() {
        clear_block(x + BLOCK_SIZE * 3, y);
        draw_block(x - BLOCK_SIZE, y);
        x -= BLOCK_SIZE;
    }

    void move_down() {
        D Debug::printf("moving down current pos : %d, %d\n", x, y);
        if (orientation == 0 || orientation == 2) {
            extend_bottom();
        } else {
            for (uint32_t block = 0; block < num_blocks; block++) {
                draw_line(x, y, x + BLOCK_SIZE * num_blocks, y, Black, nullptr);
                draw_line(x, y + BLOCK_SIZE, x + BLOCK_SIZE * num_blocks, y + BLOCK_SIZE, color, nullptr);
                y++;
            }
        }
        D Debug::printf("new pos : %d, %d\n", x, y);
    }

    void move_right() {
        D Debug::printf("moving right current pos : %d, %d\n", x, y);
        if (orientation == 0 || orientation == 2) {
            for (uint32_t block = 0; block < num_blocks; block++) {
                draw_line(x, y, x, y + BLOCK_SIZE * num_blocks - 1, Black, nullptr);
                draw_line(x + BLOCK_SIZE, y, x + BLOCK_SIZE, y + BLOCK_SIZE * num_blocks - 1, color, nullptr);
                x++;
            }
        } else {
            extend_right();
        }
        D Debug::printf("new pos : %d, %d\n", x, y);
    }

    void move_left() {
        D Debug::printf("moving left current pos : %d, %d\n", x, y);
        if (orientation == 0 || orientation == 2) {
            x -= BLOCK_SIZE;
            uint32_t start_x = x;
            for (uint32_t block = 0; block < num_blocks; block++) {
                draw_line(start_x, y, start_x, y + BLOCK_SIZE * num_blocks - 1, color, nullptr);
                draw_line(start_x + BLOCK_SIZE, y, start_x + BLOCK_SIZE, y + BLOCK_SIZE * num_blocks - 1, Black, nullptr);
                start_x++;
            }
        } else {
            extend_left();
        }
        D Debug::printf("new pos : %d, %d\n", x, y);
    }

    void rotate() {
        D Debug::printf("rotating current pos, orientation : %d, %d %d\n", x, y, orientation);
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
        D Debug::printf("after rotate current pos, orientation : %d, %d %d\n", x, y, orientation);
    }
};
*/

#endif