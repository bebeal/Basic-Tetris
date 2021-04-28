#include "processes.h"
#include "threads.h"
#include "semaphore.h"
#include "shared.h"
#include "ext2.h"
#include "io.h"
#include "u8250.h"
#include "semaphore.h"
#include "future.h"

namespace gheith {

    int PCB::sem(uint32_t count) {
        Shared<Semaphore> s = Shared<Semaphore>::make(count);
        SemaphoreDescriptor* sD = new SemaphoreDescriptor(s);
        return add_descriptor(sD);
    }

    void PCB::copy_from(Shared<PCB> from) {
        // copy virt mem
        deep_copy_mem(thread->pd, from->thread->pd);
        // copy descriptor objects, depending on descriptor dynamic type either an entirely new object is made or a deep/shallow copy, handled in copy() function per type
        for(uint32_t dti = 0; dti < NUM_DESCRIPTOR_TYPES * NUM_DESCRIPTORS; dti++) {
            if (!(from->dt[dti]->is_nothing())) {
                dt[dti] = from->dt[dti]->copy();
            }
        }
    }

    void PCB::remove_user_space() {
        delete_user_mem(thread->pd);
    }

    uint32_t* PCB::get_pd() {
        return thread->pd;
    }

    TCB* PCB::get_thread() {
        return thread;
    }

    int PCB::add_child(Shared<PCB> child) {
        ChildProcessDescriptor* cpd = new ChildProcessDescriptor(child);
        set_descriptor(child->pid, cpd);
        return child->pid;
    }

} // namepspace gheith
