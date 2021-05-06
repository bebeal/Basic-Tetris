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
bool Keyboard::caps = false;
bool Keyboard::num = false;
bool Keyboard::scroll = false;
uint8_t Keyboard::keys[256];
uint8_t Keyboard::kb_queue[BUFF_LEN];

// register interrupt vector mapping to handler
void Keyboard::init(PS2Controller* ps2C) {
    Keyboard::ps2C = ps2C;

    IDT::interrupt(PS2Controller::KBVector, (uint32_t)keyboardHandler_);
}

uint8_t Keyboard::get_ascii() {
    uint8_t get = tail;
    tail = (tail + 1) % BUFF_LEN;
    return kb_queue[get];
}

void Keyboard::handle_interrupt() {
    // See if there's room in the key buffer, else bug out.
    uint8_t nhead = (head + 1) % BUFF_LEN;
    if (nhead == tail) {
        return;
    }

    unsigned char byte = inb(0x60);

    // key release 
    if(byte & 0x80) {
        uint8_t pressed_byte = byte & 0x7F;
        // Check if we're releasing a shift key.
        if(pressed_byte == LSHIFT) {
            //Debug::printf("lshift u\n");
            shift = shift & 0x02;
        } else if(pressed_byte == RSHIFT) { 
            //Debug::printf("rshift u\n");
            shift = shift & 0x01;
        } else if(pressed_byte == CTRL) {
            //Debug::printf("ctrl u\n");
            ctrl = 0;
        }
        keys[pressed_byte] = 0;
        return;
    } 

    if(keys[byte] < 10 && keys[byte] > 0) {
        // Key is already pressed. Ignore it.
        keys[byte]++; // Increment anyway, so we can roll over and repeat. ?
        return;
    }
    keys[byte]++;

    if(byte == LSHIFT) {
        shift = shift | 0x01;
        //Debug::printf("lshift\n");
    } else if(byte == RSHIFT) {
        shift = shift | 0x02;
        //Debug::printf("rshift\n");
    } else if(byte == CTRL) {
        ctrl = 1;
        //Debug::printf("ctrl\n");
    } else if (byte == CAPS) {
        caps = !caps;
    }

    const uint8_t *codes  = lower_ascii_codes;
    if ((shift && !caps) || (!shift && caps)) {
        codes = upper_ascii_codes;
    }

    uint8_t ascii = codes[byte];
    
    if(ascii != 0) {
        kb_queue[head] = ascii;
        head = nhead;
        if (caps) {
            put_string("CAPSLOCK", 3, 2);
        } else {
            put_string("        ", 3, 2);
        }
        put_char(ascii, 3, 3);
        //Debug::printf("0x%x %c\n", byte, ascii);
        return;
    }
}

extern "C" void keyboardHandler() {
    Keyboard::handle_interrupt();
    // send EOI to the interrupt controller to acknowledge we recieved the interrupt
    SMP::eoi_reg.set(0);
}