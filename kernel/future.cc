#include "future.h"
#include "threads.h"

namespace gheith {
    template <typename Out, typename T>
    struct TaskImpl : public TCBWithStack {
        T work;
        Shared<Future<Out>> f;
    
        TaskImpl(T work, Shared<Future<Out>> f) : TCBWithStack(), work(work), f(f) {
        }

        ~TaskImpl() {
        }

        void doYourThing() override {
            Out out = work();
            f->set(out);
        }
    };
}

template <typename Out, typename T>
Shared<Future<Out>> future(T work) {
    using namespace gheith;

    delete_zombies();

    auto f = Shared<Future<Out>>::make();

    auto tcb = new TaskImpl<Out,T>(work,f);
    schedule(tcb);
    return f;
}