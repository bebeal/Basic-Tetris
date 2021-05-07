#ifndef _TETRIS_H_
#define _TETRIS_H_

#include "debug.h"
#include "graphics.h"
#include "stdint.h"
#include "pit.h"
#include "shapes.h"
#include "random.h"
#include "keyboard.h"

//#define DEBUG
#ifdef DEBUG
  #define D if(1)
#else 
  #define D if(0)
#endif

#define NONE 0
#define LEFT 'q'
#define DOWN 'w'
#define RIGHT 'e'
#define ROTATE ' '

// A, S, D, R

class Tetris {
protected:
    uint32_t con[4][2];
    uint8_t* double_buffer;
    Shape* curr_shape;
    Random* rng;
    static constexpr uint32_t GT = 1000; // game tick how long to wait before "dropping" the block, prob needs to be like 1000
    uint32_t init_x = 240;
    uint32_t init_y = 10;
    //

public:
    Tetris() {
        // top left, top right, bottom right, bottom left
        // x, y
        // TODO: change these to whatever is needed
        con[0][0] = 179;
        con[0][1] = 9;
        con[1][0] = 300;
        con[1][1] = 9;
        con[2][0] = 300;
        con[2][1] = 190;
        con[3][0] = 179;
        con[3][1] = 190;
        double_buffer = new uint8_t[WIDTH * HEIGHT];
        Keyboard::toggle_display(nullptr);
        curr_shape = new IShape(init_x, init_y, (Color) 11);
        //Debug::printf("1\n");
        curr_shape->draw_shape(double_buffer);
        //Debug::printf("2\n");
        for(uint32_t px = con[0][0]; px > con[0][0] - 4; px--) {
            draw_line(px, 10, px, 193, Red, nullptr);
        }
        for(uint32_t px = con[1][0]; px < con[1][0] + 4; px++) {
            draw_line(px, 10, px, 193, Red, nullptr);
        }
        for(uint32_t py = con[3][1]; py < con[2][1] + 4; py++) {
            draw_line(176, py, 303, py, Red, nullptr);
        }

        // Shape* show = new IShape(2, 170, (Color) 11);
        // show->draw_shape(double_buffer);
        // show = new ZShape(9, 160, (Color) 13);
        // show->draw_shape(double_buffer);
        // show = new LShape(16, 150, (Color) 2);
        // show->draw_shape(double_buffer);

        rng = new Random(0);
        //Debug::printf("3\n");
        //buffer_to_screen(double_buffer);
        // uint8_t* dir = new uint8_t[4];
        // dir[0] = LEFT;
        // dir[1] = DOWN;
        // dir[2] = RIGHT;
        // dir[3] = ROTATE;
        // for (uint8_t i = 0; i < 200; i++) {
        //     //Debug::printf("%d\n", i);
        //     move(dir[1]);
        //     //buffer_to_screen(double_buffer);
        //     sleep(1);
        // }
        // while(true) {
        //     uint8_t control_input = get_control();
        //     Debug::printf("control_input %d\n", control_input);
        //     move(control_input);
        //     sleep(1);
        // }
        //buffer_to
    }

    uint8_t get_control() {
        return Keyboard::last_press();
    }

    void next_shape() {
        delete curr_shape;
        ShapeType shape_type = (ShapeType) (rng->next() % 7);
        Color c = (Color) (Pit::jiffies % NUM_COLORS);
        if (c == Black || c == Red) {
            c = LightCyan;
        }
        switch(shape_type) {
            case ShapeType::SQUARE:
                curr_shape = new SquareShape(init_x, init_y, c);
                break;
            case ShapeType::I:
                curr_shape = new IShape(init_x, init_y, c);
                break;
            case ShapeType::L:
                curr_shape = new LShape(init_x, init_y, c);
                break;
            case ShapeType::REVERSE_L:
                curr_shape = new ReverseLShape(init_x, init_y, c);
                break;
            case ShapeType::Z:
                curr_shape = new ZShape(init_x, init_y, c);
                break;
            case ShapeType::REVERSE_Z:
                curr_shape = new ReverseZShape(init_x, init_y, c);
                break;
            case ShapeType::T:
                curr_shape = new TShape(init_x, init_y, c);
                break;
        }
        // TODO: do we want to draw it here?
        curr_shape->draw_shape(double_buffer);
        //buffer_to_screen(double_buffer);
    }

    bool clear_left() {
        uint32_t new_x = curr_shape->x - Shape::BLOCK_SIZE;
        uint32_t y = curr_shape->y;
        curr_shape->clear_shape(double_buffer);
        for (uint32_t block_index = 0; block_index < curr_shape->num_blocks; block_index++) {
            uint32_t block_num = curr_shape->on_blocks[curr_shape->orientation][block_index];
            uint32_t x_cor = to_x(new_x, block_num);
            uint32_t y_cor = to_y(y, block_num);

            // only need to check x-coordinates for bounds
            // and check if occupied
            if (x_cor <= con[0][0] || double_buffer[offset(x_cor, y_cor)] != 0) {
                curr_shape->draw_shape(double_buffer);
                return false;
            }
        }
        curr_shape->draw_shape(double_buffer);
        return true;
    }

    bool clear_right() {
        uint32_t new_x = curr_shape->x + Shape::BLOCK_SIZE;
        uint32_t y = curr_shape->y;
        // to help with checking if spaces are free
        curr_shape->clear_shape(double_buffer);
        for (uint32_t block_index = 0; block_index < curr_shape->num_blocks; block_index++) {
            uint32_t block_num = curr_shape->on_blocks[curr_shape->orientation][block_index];
            uint32_t x_cor = to_x(new_x, block_num);
            uint32_t y_cor = to_y(y, block_num);

            // only need to check x-coordinates
            // now check if occupied
            if (x_cor >= con[1][0]|| double_buffer[offset(x_cor, y_cor)] != 0) {
                curr_shape->draw_shape(double_buffer);
                return false;
            }
        }
        curr_shape->draw_shape(double_buffer);
        return true;
    }

    bool clear_down() {
        //Debug::printf("in clear_down\n");
        uint32_t x = curr_shape->x;
        uint32_t new_y = curr_shape->y + Shape::BLOCK_SIZE;
        // to help with checking if spaces are free
        curr_shape->clear_shape(double_buffer);
        for (uint32_t block_index = 0; block_index < curr_shape->num_blocks; block_index++) {
            uint32_t block_num = curr_shape->on_blocks[curr_shape->orientation][block_index];
            uint32_t x_cor = to_x(x, block_num);
            uint32_t y_cor = to_y(new_y, block_num);
            //Debug::printf("x_cor = %d\n", x_cor);
            //Debug::printf("y_cor = %d\n", y_cor);

            // only need to check y-coordinates
            //Debug::printf("got past bounds check, block_index = %d\n", block_index);
            // now check if occupied
            // but don't check the spaces where I previously was - hacky but trying to get it to work
            //curr_shape->clear_shape(double_buffer);
            if (y_cor >= con[2][1] || double_buffer[offset(x_cor, y_cor)] != Black) {
                curr_shape->draw_shape(double_buffer);
                return false;
            }
            //Debug::printf("got past occupied check, block_index = %d\n", block_index);
        }
        curr_shape->draw_shape(double_buffer);
        return true;
    }

    bool clear_rotate() {
        uint32_t x = curr_shape->x;
        uint32_t y = curr_shape->y;
        // rotate once to get new orientation
        curr_shape->rotate_templ();
        curr_shape->clear_shape(double_buffer);
        uint32_t index = 0;
        bool out_bounds = false;
        while (!out_bounds && index < curr_shape->num_blocks) {
            uint32_t new_x = to_x(x, curr_shape->on_blocks[curr_shape->orientation][index]);
            uint32_t new_y = to_y(y, curr_shape->on_blocks[curr_shape->orientation][index]);
            if (new_x <= con[0][0] || con[1][0] <= new_x) {
                out_bounds = true;
            }
            if (new_y <= con[0][1] || con[2][1] <= new_y) {
                out_bounds = true;
            }
            index++;
        }

        // rotate 3 more times to go back
        for (int i = 0; i < 3; i++) {
            curr_shape->rotate_templ();
        }
        curr_shape->draw_shape(double_buffer);
        return out_bounds;
    }

    // returns true if the row is complete and can 
    /*
    bool row_complete(uint32_t y) {
        uint32_t begin_x = con[0][0] + 1;
    }
    */

    // collision check, if valid, move shape
    bool move(uint8_t move) {
        if (move == 0) { // keyboard input wasn't a valid move (i.e. they pressed like 9 or 'H' or something random)
            return false;
        }
        bool update_screen = false;
        switch(move) {
            case LEFT: 
                if (true) {
                    //Debug::printf("kdfjsfjs\n");
                    curr_shape->move_left(double_buffer);
                    update_screen = true;
                }
                break;
            case DOWN:
                if (clear_down()) {
                    curr_shape->move_down(double_buffer);
                    update_screen = true;
                }
                break;
            case RIGHT:
                if (true) {
                    curr_shape->move_right(double_buffer);
                    update_screen = true;
                }
                break;
            case ROTATE:
                if (true) {
                    curr_shape->rotate(double_buffer);
                    update_screen = true;
                }
                break;
        }
        //if (update_screen) {
            //buffer_to_screen(double_buffer);
            // check if any rows filled
        //}
        return update_screen;
    }

    void play_game() {
        // between every GT number of jiffies, we'll force a call to move_down() on the shape
        volatile uint32_t time_drop = Pit::jiffies + GT;
        while(true) {
            //sleep(1);
            if (Pit::jiffies >= time_drop) {
                bool moved_down = move(DOWN);
                if (!moved_down) {
                    next_shape();
                }
                
                time_drop = Pit::jiffies + GT;
            }
            uint8_t control_input = get_control();
            //Debug::printf("control_input %d\n", control_input);
            move(control_input);
        }
    }
};

#endif