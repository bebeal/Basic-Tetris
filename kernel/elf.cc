#include "elf.h"
#include "debug.h"

uint32_t ELF::load(Shared<Node> file) {
    if (file != nullptr) {
        ElfHeader hdr;

        file->read(0,hdr);

        if (((uint32_t)hdr.magic0) != 0x7f || hdr.magic1 != 'E' || hdr.magic2 != 'L' || hdr.magic3 != 'F') {
            return 0;
        }

        uint32_t hoff = hdr.phoff;

        for (uint32_t i=0; i<hdr.phnum; i++) {
            ProgramHeader phdr;
            file->read(hoff,phdr);
            hoff += hdr.phentsize;

            if (phdr.type == 1) { // 1 = load
                char *p = (char*) phdr.vaddr;
                uint32_t memsz = phdr.memsz;
                uint32_t filesz = phdr.filesz;

                //Debug::printf("vaddr:%x memsz:0x%x filesz:0x%x fileoff:%x\n",p,memsz,filesz,phdr.offset);
                file->read_all(phdr.offset,filesz,p);
                bzero(p + filesz, memsz - filesz); // zero out any uninitialized data, file doesn't store additional 0's for them
            }
        }
        return hdr.entry;
    }
    return 0;
}



















































#if 0
    ElfHeader hdr;

    file->read(0,hdr);

    uint32_t hoff = hdr.phoff;

    for (uint32_t i=0; i<hdr.phnum; i++) {
        ProgramHeader phdr;
        file->read(hoff,phdr);
        hoff += hdr.phentsize;

        if (phdr.type == 1) {
            char *p = (char*) phdr.vaddr;
            uint32_t memsz = phdr.memsz;
            uint32_t filesz = phdr.filesz;

            Debug::printf("vaddr:%x memsz:0x%x filesz:0x%x fileoff:%x\n",
                p,memsz,filesz,phdr.offset);
            file->read_all(phdr.offset,filesz,p);
            bzero(p + filesz, memsz - filesz);
        }
    }

    return hdr.entry;
#endif
