// Reference https://github.com/knusbaum/kernel/blob/master/keyboard.c

#include "keyboard.h"
#include "debug.h"
#include "machine.h"
#include "smp.h"
#include "scancode.h"
#include "graphics.h"
#include "fonts.h"

PS2Controller* Keyboard::ps2C = 0;
uint8_t Keyboard::head = 0;
uint8_t Keyboard::tail = 0;
uint8_t Keyboard::shift = 0;
uint8_t Keyboard::ctrl = 0;
bool volatile Keyboard::caps = false;
bool volatile Keyboard::num = false;
bool volatile Keyboard::scroll = false;
uint8_t Keyboard::keys[256];
uint8_t Keyboard::kb_queue[BUFF_LEN];
bool volatile Keyboard::display = false;

// register interrupt vector mapping to handler
void Keyboard::init(PS2Controller* ps2C) {
    Keyboard::ps2C = ps2C;

    IDT::interrupt(PS2Controller::KBVector, (uint32_t)keyboardHandler_);
}

uint8_t Keyboard::last_press() {
    if (tail == head) {
        return 0;
    }
    uint32_t c_tail = tail;
    tail = (tail + 1) % BUFF_LEN;
    return kb_queue[c_tail];
}

void Keyboard::toggle_display(uint8_t* double_buffer) {
    // display these 5 lines on the screen starting with the top left K pixel at (3, 1)
    // Keyboard State
    // --------------
    // Toggled: CAPSLOCK 
    // Holding: LSHIFT + RSHIFT + CTRL +
    // Key: 
    if (display) {
        // toggled and was previously on, clear screen
        for(uint32_t line = 1; line <= 5; line++) {
            draw_line(1, line, 30, line, Black, double_buffer);
        }
    } else {
        put_string("Keyboard State:", 2, 1, White, Black, double_buffer);
        put_string("--------------", 2, 2, White, Black, double_buffer);
        put_string("Toggled: ", 2, 3, White, Black, double_buffer);
        put_string("Holding: ", 2, 4, White, Black, double_buffer);
        put_string("Key: ", 2, 5, White, Black, double_buffer);
    }
    display = !display;
}

void Keyboard::display_(const char* str, uint32_t cx, uint32_t cy) {
    if (display) {
        put_string(str, cx, cy, White, Black);
    }
}

void Keyboard::display_(uint8_t ascii, uint32_t cx, uint32_t cy) {
    if (display) {
        put_char(ascii, cx, cy, White, Black);
    }
}

void Keyboard::handle_interrupt() {
    // overwrite buffer if full
    uint8_t nhead = (head + 1) % BUFF_LEN;

    unsigned char byte = inb(0x60);
    // key release 
    if(byte & 0x80) {
        uint8_t pressed_byte = byte & 0x7F;
        // Check if we're releasing a shift key.
        if(pressed_byte == LSHIFT) {
            display_("        ", 11, 4);
            shift = shift & 0x02;
        } else if(pressed_byte == RSHIFT) { 
            display_("        ", 11, 4);
            shift = shift & 0x01;
        } else if(pressed_byte == CTRL) {
            display_("      ", 11, 4);
            ctrl = 0;
        } else {
            display_(" ", 7, 5);
        }
        keys[pressed_byte] = 0;
        return;
    } 

    if(keys[byte] < 10 && keys[byte] > 0) {
        // Key is already pressed. Ignore it.
        keys[byte]++; // Increment anyway, so we can roll over and repeat. 
        return;
    }
    keys[byte]++;
    bool changed = caps;
    if(byte == LSHIFT) {
        display_("LSHIFT +", 11, 4);
        shift = shift | 0x01;
    } else if(byte == RSHIFT) {
        display_("RSHIFT +", 11, 4);
        shift = shift | 0x02;
    } else if(byte == CTRL) {
        display_("CTRL +", 11, 4);
        ctrl = 1;
    } else if (byte == CAPS) {
        caps = !caps;
    }

    const uint8_t *codes  = lower_ascii_codes;
    if ((shift && !caps) || (!shift && caps)) {
        codes = upper_ascii_codes;
    }

    if (caps != changed) {
        if (caps) {
            display_("CAPSLOCK", 11, 3);
        } else {
            display_("        ", 11, 3);
        }
    }

    uint8_t ascii = codes[byte];
    
    if(ascii != 0) {
        kb_queue[head] = ascii;
        head = nhead;
        display_(ascii, 7, 5);
    }
}

extern "C" void keyboardHandler() {
    Keyboard::handle_interrupt();
    SMP::eoi_reg.set(0);
}



