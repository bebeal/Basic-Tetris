#include "debug.h"
#include "ide.h"
#include "ext2.h"
#include "kernel.h"
#include "elf.h"
#include "machine.h"
#include "libk.h"
#include "config.h"
#include "sys.h"

const char* initName = "/sbin/init";

namespace bebeal {
    Shared<Ext2> root_fs = Shared<Ext2>::make(Shared<Ide>::make(1));
    // root_fs->find->(root_fs->root, initName);
}

void kernelMain(void) {
    auto argv = new const char* [2];
    argv[0] = "init";
    argv[1] = nullptr;
    
    int rc = SYS::exec(initName,1,argv);
    Debug::panic("*** rc = %d",rc);
}

