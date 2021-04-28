#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

#include "stdint.h"
#include "atomic.h"
#include "queue.h"
#include "shared.h"

namespace gheith {
	struct TCB;
}

class Semaphore {
    uint64_t volatile count;
    ISL lock;
    Queue<gheith::TCB,NoLock> waiting;
public:
    Atomic<uint32_t> ref_count;
    Semaphore(const uint32_t count) : count(count), lock(), waiting(), ref_count(0) {}

    Semaphore(const Semaphore&) = delete;

    void down();

    void up();

    friend class Shared<Semaphore>;
    
};

#endif

