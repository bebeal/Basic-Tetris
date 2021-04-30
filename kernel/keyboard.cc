#include "keyboard.h"
#include "debug.h"
#include "machine.h"
#include "smp.h"

U8042* keyboard::ps2C = 0;

// register interrupt vector mapping to handler
void keyboard::init(U8042* ps2C) {
    keyboard::ps2C = ps2C;

    IDT::interrupt(U8042::KBVector, (uint32_t)keyboardHandler_);
}

extern "C" void keyboardHandler() {
    // read from data port
    char c = inb(0x60);
    // send EOI to the interrupt controller to acknowledge we recieved the interrupt
    //SMP::eoi_reg.set(0);
    outb(0x20, 0x20);
    Debug::printf("in keyboardHandler, char: %c\n", c);
}