#ifndef _VMM_H_
#define _VMM_H_

#include "stdint.h"

namespace gheith {
    static constexpr uint32_t ENTRIES = 1024;
    static constexpr uint32_t ADDR_SIZE = 4;
    extern void map(uint32_t* pd, uint32_t va, uint32_t pa);
    extern void unmap(uint32_t* pd, uint32_t va);
    extern uint32_t* make_pd();
    extern void delete_pd(uint32_t* pd);
    uint32_t* make_entry(uint32_t* page, uint32_t index);
    void deep_copy_mem(uint32_t* pdTo, uint32_t* pdFrom);
    void delete_user_mem(uint32_t* pd);
    uint32_t* page_mask(uint32_t page_entry);
    uint32_t get_va(uint32_t pdi, uint32_t pti);
}

namespace VMM {

    // Called (on the initial core) to initialize data structures, etc
    extern void global_init();

    // Called on each core to do per-core initialization
    extern void per_core_init();
}

#endif
