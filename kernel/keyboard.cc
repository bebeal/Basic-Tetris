#include "keyboard.h"
#include "debug.h"
#include "machine.h"
#include "smp.h"

char U8042::get() {
    char x = inb(PS2::DATA_PORT); 
    return x;
}

void U8042::put(char c) {
    outb(PS2::DATA_PORT,c);
}

// register interrupt vector mapping to handler
void U8042::init() {
    IDT::interrupt(U8042::APIT_keyboard_vector, (uint32_t)keyboardHandler_);
}

extern "C" void keyboardHandler(char c) {
    // send EOI to the interrupt controller to acknowledge we recieved the interrupt
    SMP::eoi_reg.set(0);
    Debug::printf("in keyboardHandler, char: %c\n", c);
}

