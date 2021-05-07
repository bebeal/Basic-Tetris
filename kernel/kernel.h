#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "shared.h"
#include "ext2.h"
#include "graphics.h"
#include "fonts.h"
#include "keyboard.h"

void kernelMain(void);

namespace bebeal {
    extern Shared<Ext2> root_fs;
}

#endif
