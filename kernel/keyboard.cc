// Reference https://github.com/knusbaum/kernel/blob/master/keyboard.c

#include "keyboard.h"
#include "debug.h"
#include "machine.h"
#include "smp.h"
#include "scancode.h"

PS2Controller* Keyboard::ps2C = 0;
uint8_t Keyboard::head = 0;
uint8_t Keyboard::tail = 0;
uint8_t Keyboard::shift = 0;
uint8_t Keyboard::ctrl = 0;
uint8_t Keyboard::keys[256];
uint8_t Keyboard::kb_queue[BUFF_LEN];

// register interrupt vector mapping to handler
void Keyboard::init(PS2Controller* ps2C) {
    Keyboard::ps2C = ps2C;

    IDT::interrupt(PS2Controller::KBVector, (uint32_t)keyboardHandler_);
}

uint8_t Keyboard::get_key() {
    return 0;
}

uint8_t Keyboard::get_ascii(uint8_t key) {
    return 0;
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
            shift = shift & 0x02;
        } else if(pressed_byte == RSHIFT) { 
            shift = shift & 0x01;
        } else if(pressed_byte == CTRL) {
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
    } else if(RSHIFT == 0x36) {
        shift = shift | 0x02;
    } else if(byte == CTRL) {
        ctrl = 1;
    }

    const uint8_t *codes  = lower_ascii_codes;
    if (shift) {
        codes = upper_ascii_codes;
    }

    uint8_t ascii = codes[byte];
    
    if(ascii != 0) {
        kb_queue[head] = ascii;
        head = nhead;
        return;
    }
}

extern "C" void keyboardHandler() {
    // read from data port
    unsigned char c = inb(0x60);
    // send EOI to the interrupt controller to acknowledge we recieved the interrupt
    SMP::eoi_reg.set(0);
    Debug::printf("keypressed as hex: 0x%x\n", c);
}