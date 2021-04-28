#include "debug.h"
#include "smp.h"
#include "debug.h"
#include "config.h"
#include "machine.h"
#include "ext2.h"
#include "shared.h"
#include "threads.h"
#include "vmm.h"
#include "processes.h"



namespace gheith {
    Atomic<uint32_t> TCB::next_id{0};

    TCB** activeThreads;
    TCB** idleThreads;

    Queue<TCB,InterruptSafeLock> readyQ{};
    Queue<TCB,InterruptSafeLock> zombies{};

    TCB* current() {
        auto was = Interrupts::disable();
        TCB* out = activeThreads[SMP::me()];
        Interrupts::restore(was);
        return out;
    }

    void entry() {
        auto me = current();
        vmm_on((uint32_t)me->pd);
        sti();
        me->doYourThing();
        stop();
    }

    void delete_zombies() {
        while (true) {
            auto it = zombies.remove();
            if (it == nullptr) return;
            delete it;
        }
    }

    void schedule(TCB* tcb) {
        if (!tcb->isIdle) {
            readyQ.add(tcb);
        }
    }

    struct IdleTcb: public TCB {
        IdleTcb(): TCB(true) {}
        void doYourThing() override {
            Debug::panic("should not call this");
        }
        uint32_t interruptEsp() override {
            // idle threads never enter user mode, this should be ok
            return 0;
        }
    };

    TCB::TCB(bool isIdle) : isIdle(isIdle), id(next_id.fetch_add(1)) {
        // since one for one we'll just make the pid the thread id
        pcb = Shared<PCB>::make(this->next_id.get(), this);
        saveArea.tcb = this;
        pd = make_pd();

        saveArea.cr3 = (uint32_t) pd;
    }
// wtf ?
    TCB::TCB(bool isIdle, uint32_t pid) : isIdle(isIdle), id(next_id.fetch_add(1)) {
        saveArea.tcb = this;
        pd = make_pd();

        saveArea.cr3 = (uint32_t) pd;
        // since one for one we'll just make the pid the thread id
        pcb = Shared<PCB>::make(pid, this);
    }

    TCB::~TCB() {
        delete_pd(pd);
    }
}; // namespace gheith

void threadsInit() {
    using namespace gheith;
    activeThreads = new TCB*[kConfig.totalProcs]();
    idleThreads = new TCB*[kConfig.totalProcs]();

    // swiched to using idle threads in order to discuss in class
    for (unsigned i=0; i<kConfig.totalProcs; i++) {
        idleThreads[i] = new IdleTcb();
        activeThreads[i] = idleThreads[i];
    }

    // The reaper
    thread([] {
        //Debug::printf("| starting reaper\n");
        while (true) {
            ASSERT(!Interrupts::isDisabled());
            delete_zombies();
            yield();
        }
    });
    
}

void yield() {
    using namespace gheith;
    block(BlockOption::CanReturn,[](TCB* me) {
        schedule(me);
    });
}

void stop() {
    using namespace gheith;

    while(true) {
        block(BlockOption::MustBlock,[](TCB* me) {
            if (!me->isIdle) {
                zombies.add(me);
            }
        });
        ASSERT(current()->isIdle);
    }
}
