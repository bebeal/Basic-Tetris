#include "keyboard.h"
#include "debug.h"

// IRQ1: read from IO Port 0x60
// send EOI to interrupt controller and retur from interrupt handler
char U8042::get() {
    char x = inb(PS2::DATA_PORT); 
    return x;
}

void U8042::put(char c) {
    outb(PS2::DATA_PORT,c);
}

