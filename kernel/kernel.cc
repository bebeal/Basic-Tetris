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
#include "machine.h"
#include "graphics.h"
#include "tetris.h"
#include "pit.h"

const char* initName = "/sbin/init";

namespace bebeal {
    Shared<Ext2> root_fs = Shared<Ext2>::make(Shared<Ide>::make(1));
    // root_fs->find->(root_fs->root, initName);
    
}

void kernelMain(void) {
    // emulates a keyboard interrupt
    Debug::printf("Made it to kernel main!\n");

    IShape i = IShape(25, 25, (Color) 11);
    sleep(1);
    i.move_down();
    sleep(1);
    i.move_down();
    sleep(1);
    i.move_right();
    sleep(1);
    i.move_right();
    sleep(1);
    i.move_left();
    sleep(1);
    i.move_left();
    sleep(1);
    i.rotate();
    sleep(1);
    i.move_down();
    sleep(1);
    i.move_right();
    sleep(1);
    i.move_left();
    sleep(1);
    i.rotate();
    sleep(1);
    i.move_down();
    sleep(1);
    i.move_right();
    sleep(1);
    i.move_left();
    sleep(1);
    while(true);
}

