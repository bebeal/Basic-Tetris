#include "shapes.h"

//static const uint32_t SQUARE_P[4][4] = {{5, 6, 9, 10}, {5, 6, 9, 10}, {5, 6, 9, 10}, {5, 6, 9, 10}};
//static const uint32_t I_P[4][4] = {{1, 5, 9, 13}, {4, 5, 6, 7}, {1, 5, 9, 13}, {4, 5, 6, 7}};
//static const uint32_t L_P[4][4] = {{4, 8, 12, 13}, {4, 5, 6, 8}, {5, 6, 10, 14}, {6, 8, 9, 10}};
//static const uint32_t REV_L_P[4][4] = {{5, 9, 12, 13}, {4, 8, 9, 10}, {4, 5, 8, 12}, {8, 9, 10, 14}};
//static const uint32_t Z_P[4][4] = {{8, 9, 13, 14}, {5, 8, 9, 12}, {8, 9, 13, 14}, {5, 8, 9, 12}};
//static const uint32_t REV_Z_P[4][4] = {{9, 10, 12, 13}, {5, 9, 10, 14}, {9, 10, 12, 13}, {5, 9, 10, 14}};
//static const uint32_t T_P[4][4] = {{4, 5, 6, 9}, {6, 9, 10, 14}, {9, 12, 13, 14}, {4, 8, 9, 12}};

uint32_t to_x(uint32_t global_x, uint32_t block_num) {
    //Debug::printf("global_x = %d\n", global_x);
    return global_x + ((block_num % 4) << 2);
}
uint32_t to_y(uint32_t global_y, uint32_t block_num) {
    //Debug::printf("global_y = %d\n", global_y);
    //Debug::printf("block_num >> 2 %d\n", block_num >> 2);
    //Debug::printf()
    return global_y + ((block_num >> 2) << 2);
}

void Shape::rotate_templ() {
    orientation = (orientation + 1) % NUM_ORIENTATIONS;
}

// remember that x is columns and y is rows
uint32_t Shape::to_x(uint32_t block_num) {
    return ::to_x(x, block_num);
}

uint32_t Shape::to_y(uint32_t block_num) {
    return ::to_y(y, block_num);
}

void Shape::clear_shape(uint8_t* double_buffer) {
    // clear each block in the shape
    for (uint32_t block_index = 0; block_index < num_blocks; block_index++) {
        uint32_t x_cor = to_x(on_blocks[orientation][block_index]);
        uint32_t y_cor = to_y(on_blocks[orientation][block_index]);
        fill_block(x_cor, y_cor, Black, double_buffer);
        fill_block(x_cor, y_cor, Black, nullptr);
    }
}

void Shape::draw_shape(uint8_t* double_buffer) {
    // draw each block in the shape
    for (uint32_t block_index = 0; block_index < num_blocks; block_index++) {
        uint32_t x_cor = to_x(on_blocks[orientation][block_index]);
        uint32_t y_cor = to_y(on_blocks[orientation][block_index]);
        //uint32_t block_num = on_blocks[orientation][block_index];
        /*
        Debug::printf("block_num = %d\n", block_num);
        Debug::printf("x_cor = %d\n", x_cor);
        Debug::printf("y_cor = %d\n", y_cor);
        Debug::printf("double_buffer %x\n", double_buffer);
        */
        fill_block(x_cor, y_cor, color, double_buffer);
        fill_block(x_cor, y_cor, color, nullptr);
    }
}

void Shape::move_down(uint8_t* double_buffer) {
    clear_shape(double_buffer);
    y += BLOCK_SIZE;
    draw_shape(double_buffer);
}

void Shape::move_right(uint8_t* double_buffer) {
    clear_shape(double_buffer);
    x += BLOCK_SIZE;
    draw_shape(double_buffer);
}

void Shape::move_left(uint8_t* double_buffer) {
    clear_shape(double_buffer);
    x -= BLOCK_SIZE;
    draw_shape(double_buffer);
}

void Shape::rotate(uint8_t* double_buffer) {
    clear_shape(double_buffer);
    //Debug::printf("finished clearing\n");
    rotate_templ();
    //Debug::printf("finished rotating template\n");
    draw_shape(double_buffer);
}

void Shape::fill_block(uint32_t x_start, uint32_t y_start, Color c, uint8_t* double_buffer) {
    for (uint32_t line = 0; line < BLOCK_SIZE; line++) {
        draw_line(x_start, y_start + line, x_start + 4, y_start + line, c, double_buffer);
    }
}

SquareShape::SquareShape(uint32_t x, uint32_t y, Color color) : Shape(ShapeType::SQUARE, 0, 4, x, y) {
    uint32_t SQUARE_P[4][4] = {{5, 6, 9, 10}, {5, 6, 9, 10}, {5, 6, 9, 10}, {5, 6, 9, 10}};
    for (uint32_t i = 0; i < NUM_ORIENTATIONS; i++) {
        for (uint32_t j = 0; j < num_blocks; j++) {
            on_blocks[i][j] = SQUARE_P[i][j];
        }
    }
    for(uint32_t i = 0; i < 4; i++) {
        //Debug::printf("onblock[%d] = %d, %d\n", i, on_blocks[orientation][i], SQUARE_P[orientation][i]);
    }
}
/* 
void SquareShape::rotate_templ() {
    Debug::printf("entering SquareShape::rotate_templ\n");
    orientation = (orientation + 1) % NUM_ORIENTATIONS;
    Debug::printf("orientation = %d\n", orientation);
    Debug::printf("modified on_blocks\n");
    for(uint32_t i = 0; i < 4; i++) {
        Debug::printf("onblock[%d] = %d\n", i, on_blocks[orientation][i]);
    }
}
 */
IShape::IShape(uint32_t x, uint32_t y, Color color) : Shape(ShapeType::SQUARE, 0, 4, x, y) {
    uint32_t I_P[4][4] = {{1, 5, 9, 13}, {4, 5, 6, 7}, {1, 5, 9, 13}, {4, 5, 6, 7}};
    for (uint32_t i = 0; i < NUM_ORIENTATIONS; i++) {
        for (uint32_t j = 0; j < num_blocks; j++) {
            on_blocks[i][j] = I_P[i][j];
        }
    }
}
/* 
void IShape::rotate_templ() {
    orientation = (orientation + 1) % NUM_ORIENTATIONS;
    *on_blocks = (uint32_t*) &(I_P[orientation]);
}
 */
LShape::LShape(uint32_t x, uint32_t y, Color color) : Shape(ShapeType::SQUARE, 0, 4, x, y) {
    uint32_t L_P[4][4] = {{4, 8, 12, 13}, {4, 5, 6, 8}, {5, 6, 10, 14}, {6, 8, 9, 10}};
    for (uint32_t i = 0; i < NUM_ORIENTATIONS; i++) {
        for (uint32_t j = 0; j < num_blocks; j++) {
            on_blocks[i][j] = L_P[i][j];
        }
    }
}
/* 
void LShape::rotate_templ() {
    orientation = (orientation + 1) % NUM_ORIENTATIONS;
    *on_blocks = (uint32_t*) &(L_P[orientation]);
}
 */
ReverseLShape::ReverseLShape(uint32_t x, uint32_t y, Color color) : Shape(ShapeType::SQUARE, 0, 4, x, y) {
    uint32_t REV_L_P[4][4] = {{5, 9, 12, 13}, {4, 8, 9, 10}, {4, 5, 8, 12}, {8, 9, 10, 14}};
    for (uint32_t i = 0; i < NUM_ORIENTATIONS; i++) {
        for (uint32_t j = 0; j < num_blocks; j++) {
            on_blocks[i][j] = REV_L_P[i][j];
        }
    }
}
/* 
void ReverseLShape::rotate_templ() {
    orientation = (orientation + 1) % NUM_ORIENTATIONS;
    *on_blocks = (uint32_t*) &(REV_L_P[orientation]);
}
 */
ZShape::ZShape(uint32_t x, uint32_t y, Color color) : Shape(ShapeType::SQUARE, 0, 4, x, y) {
    uint32_t Z_P[4][4] = {{8, 9, 13, 14}, {5, 8, 9, 12}, {8, 9, 13, 14}, {5, 8, 9, 12}};
    for (uint32_t i = 0; i < NUM_ORIENTATIONS; i++) {
        for (uint32_t j = 0; j < num_blocks; j++) {
            on_blocks[i][j] = Z_P[i][j];
        }
    }
}
/* 
void ZShape::rotate_templ() {
    orientation = (orientation + 1) % NUM_ORIENTATIONS;
    *on_blocks = (uint32_t*) &(Z_P[orientation]);
}
 */
ReverseZShape::ReverseZShape(uint32_t x, uint32_t y, Color color) : Shape(ShapeType::SQUARE, 0, 4, x, y) {
    uint32_t REV_Z_P[4][4] = {{9, 10, 12, 13}, {5, 9, 10, 14}, {9, 10, 12, 13}, {5, 9, 10, 14}};
    for (uint32_t i = 0; i < NUM_ORIENTATIONS; i++) {
        for (uint32_t j = 0; j < num_blocks; j++) {
            on_blocks[i][j] = REV_Z_P[i][j];
        }
    }
}
/* 
void ReverseZShape::rotate_templ() {
    orientation = (orientation + 1) % NUM_ORIENTATIONS;
    *on_blocks = (uint32_t*) &(REV_Z_P[orientation]);
}
 */
TShape::TShape(uint32_t x, uint32_t y, Color color) : Shape(ShapeType::SQUARE, 0, 4, x, y) {
    uint32_t T_P[4][4] = {{4, 5, 6, 9}, {6, 9, 10, 14}, {9, 12, 13, 14}, {4, 8, 9, 12}};
    for (uint32_t i = 0; i < NUM_ORIENTATIONS; i++) {
        for (uint32_t j = 0; j < num_blocks; j++) {
            on_blocks[i][j] = T_P[i][j];
        }
    }
}

/* void TShape::rotate_templ() {
    orientation = (orientation + 1) % NUM_ORIENTATIONS;
    *on_blocks = (uint32_t*) &(T_P[orientation]);
} */