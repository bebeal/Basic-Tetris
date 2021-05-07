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
#include "fonts.h"
#include "tetris.h"

const char* initName = "/sbin/init";

namespace bebeal {
    Shared<Ext2> root_fs = Shared<Ext2>::make(Shared<Ide>::make(1));
    // root_fs->find->(root_fs->root, initName);
    
}

void kernelMain(void) {
    Debug::printf("Made it to kernel main!\n");
    Tetris game = Tetris();
    game.play_game();

    //Keyboard::toggle_display(nullptr);
    while(true);
}

