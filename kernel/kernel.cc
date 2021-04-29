#include "debug.h"
#include "ide.h"
#include "ext2.h"
#include "kernel.h"
#include "elf.h"
#include "machine.h"
#include "libk.h"
#include "config.h"
#include "sys.h"
#include "keyboard.h"

const char* initName = "/sbin/init";

namespace bebeal {
    Shared<Ext2> root_fs = Shared<Ext2>::make(Shared<Ide>::make(1));
    // root_fs->find->(root_fs->root, initName);
    
}

void kernelMain(void) {
    while((inb(PS2::STAT_COMM_PORT) & 0x2));
    outb(PS2::STAT_COMM_PORT, 0xD2);
    while((inb(PS2::STAT_COMM_PORT) & 0x2));
    outb(PS2::DATA_PORT, 'a');
    while((inb(PS2::STAT_COMM_PORT) & 0x2));
    outb(PS2::STAT_COMM_PORT, 0xD2);
    while((inb(PS2::STAT_COMM_PORT) & 0x2));
    outb(PS2::DATA_PORT, 0xFA);
    Debug::printf("Made it to kernel main!\n");
}

