#include "sys.h"
#include "stdint.h"
#include "idt.h"
#include "debug.h"
#include "machine.h"
#include "config.h"
#include "shared.h"
#include "processes.h"
#include "future.h"
#include "semaphore.h"
#include "ext2.h"
#include "libk.h"
#include "vmm.h"
#include "kernel.h"
#include "elf.h"
#include "physmem.h"

void SYS::init(void) {
    IDT::trap(48,(uint32_t)sysHandler_,3);
}

void SYS::exit(Shared<gheith::PCB> process, int rc) {
    process->exit(rc);
    stop();
}

int SYS::open(Shared<gheith::PCB> process, const char* fn, int flags) {
    return process->open(fn);
}

ssize_t SYS::len(Shared<gheith::PCB> process, int fd) {
    return process->len(fd);
}

ssize_t SYS::write(Shared<gheith::PCB> process, int fd, void* buf, size_t nbyte) {
    return process->write(fd, (char*) buf, nbyte);
}

ssize_t SYS::read(Shared<gheith::PCB> process, int fd, void* buf, size_t nbyte) {
    return process->read(fd, buf, nbyte);
}

int SYS::sem(Shared<gheith::PCB> process, uint32_t count) {
    return process->sem(count);
}

int SYS::up(Shared<gheith::PCB> process, int sd) {
    return process->up(sd);
}

int SYS::down(Shared<gheith::PCB> process, int sd) {
    return process->down(sd);
}

int SYS::close(Shared<gheith::PCB> process, int id) {
    return process->close(id);
}

int SYS::shutdown(void) {
    Debug::shutdown();
    return -1;
}

int SYS::wait(Shared<gheith::PCB> process, int cd, uint32_t *status) {
    int child_status = process->wait(cd);
    *status = child_status;
    return child_status;
}

off_t SYS::seek(Shared<gheith::PCB> process, int fd, off_t offset) {
    return process->seek(fd, offset);
}

int SYS::fork(Shared<gheith::PCB> process, uint32_t pc, uint32_t esp) {
    uint32_t child_pid = process->reserve_child_descriptor();
    if (child_pid < 0) {
        return child_pid;
    }
    // create child process
    Shared<gheith::PCB> child_process = thread([pc, esp] {
            switchToUser(pc, esp, 0);
    }, child_pid, process);
    process->add_child(child_process);
    return child_pid;
}

void SYS::delete_args(const char* path, uint32_t argc, const char** argv) {
    // clear up mem
    for(uint32_t arg_i = 0; arg_i < argc; arg_i++) {
        delete[] argv[arg_i];
    }
    delete[] argv;
    delete[] path;
}

// call delete_args to clear the stuff we added to the heap in this method
int SYS::save_args_call_exec(uint32_t* user_stack) {
    uint32_t i = 2; // start at 2 cause we want to start at arg0... execl(const char* path, const char* arg0, ....);
    uint32_t argc = 0;
    while((char*)user_stack[i] != nullptr) {
        argc++;
        i++;
    }
    auto argv = new const char*[argc + 1];
    argv[argc] = nullptr;
    //uint32_t* arg_lens = new uint32_t[argc];
    // allocate stuff on kernel exist after we wipe memory
    for(uint32_t arg = 0; arg < argc; arg++) {
        char* current_arg = (char*) user_stack[arg + 2]; // + 2 to start at arg0
        //Debug::printf("%s\n", current_arg);
        uint32_t arg_len = K::strlen(current_arg) + 1; // + 1 for nullterminated string
        uint32_t byte_aligned_arg_len = ((arg_len + 3) / 4) * 4; // 4 byte align length of string
        argv[arg] = new char[byte_aligned_arg_len];
        memcpy((void*) argv[arg], current_arg, arg_len);
    }
    const char* path = new char[K::strlen((char*) user_stack[1]) + 1];
    memcpy((void*)path, (void*) user_stack[1], K::strlen((char*) user_stack[1]) + 1);
    return SYS::exec(path, argc, argv,true);
}

void SYS::setup_exec_stack(int argc, const char** argv, uint32_t userEsp, uint32_t bytes_needed, uint32_t* arg_lens, uint32_t* pd) {
    using namespace gheith;
    auto va = userEsp;
    // allocate room on user stack for args for main
    for(uint32_t bytes_allocated = 0; bytes_allocated < bytes_needed + 4; bytes_allocated += 4096) {
        uint32_t pa = PhysMem::alloc_frame();
        map(pd, va, pa);
        va += 4096;
    }

    // add strings to stack and save addresses to store as "argv" pointer later. making our way up the stack with smaller memory addressses
    uint32_t* addrs = new uint32_t[argc];
    uint32_t* esp = (uint32_t*) (userEsp + bytes_needed); // at the bottom of stack with bigger memory address
    for(int arg_i = argc - 1; arg_i >= 0; arg_i--) {
        const char* current_arg = argv[arg_i];
        esp -= (arg_lens[arg_i]); // correct esp
        addrs[arg_i] = (uint32_t) esp;
        memcpy((void*)esp, current_arg, arg_lens[arg_i] * 4); // * 4 cause memcpy wants in bytes
    }

    // null terminate the argv pointer
    esp--; 
    *esp = (uint32_t)nullptr;
    // add addrs to string as "argv" array
    for(int arg_i = argc - 1; arg_i >= 0; arg_i--) {
        esp--;
        *esp = addrs[arg_i];
    }
    // argv pointer itself (const char**)
    esp--;
    *esp = (uint32_t) (esp + 1);
    // argc
    esp--;
    *esp = argc;
    delete[] addrs;
}

// assume stuff saved on heap if needed to be saved on heap
int SYS::exec(const char* path, int argc, const char** argv, bool delete_args) {
    using namespace gheith;
    TCB* me = current();
    uint32_t* pd = me->pd;
    Shared<PCB> process = me->pcb;

    // wipe user memory 
    process->remove_user_space();

    // load elf binary
    Shared<Node> init = bebeal::root_fs->find(bebeal::root_fs->root, path);
    uint32_t e = ELF::load(init);
    // ensure it's an elf and not in kernel space
    if (e == 0 || e < 0x80000000) {
        if (delete_args) {
            SYS::delete_args(path, argc, argv);
        }
        return -1;
    }

    // byte align strings and count how many bytes we need for the stack
    uint32_t bytes_needed = 4 + (4 * (argc + 1)) + 4; // starts at 4 for argc and + (4 * (argc + 1)) to account for pointers for each arg + the nullptr + 4 for pointer argv itself
    uint32_t* arg_lens = new uint32_t[argc]; // store how many memory addresses each string will need to be properly placed on the stack
    for(int arg_i = 0; arg_i < argc; arg_i++) {
        uint32_t arg_len = K::strlen(argv[arg_i]) + 1; // + 1 for null terminator
        uint32_t byte_aligned_arg_len = ((arg_len + 3) / 4) * 4; // byte aligns
        arg_lens[arg_i] = byte_aligned_arg_len / 4; // how many uint32_t's/memory addrs this string will take up
        bytes_needed += byte_aligned_arg_len; 
    }

    // setup stack and switch to user pointing to userEsp
    auto userEsp = 0xefffe000; 
    SYS::setup_exec_stack(argc, argv, userEsp, bytes_needed, arg_lens, pd);
    if (delete_args) {
        SYS::delete_args(path, argc, argv);
    }
    switchToUser(e, userEsp, 0);
    return -1;
}

// version that doesn't save args to kernel heap before wiping memory
int SYS::exec(const char* path, int argc, const char** argv) {
    return SYS::exec(path, argc, argv, false);
}


extern "C" int sysHandler(uint32_t eax, uint32_t *frame) {
    using namespace gheith;
    
    // user_stack[0] points to user's RA from this sys call, params follow thereafter
    uint32_t* user_stack = (uint32_t*) frame[3];
    // parent process
    Shared<PCB> process = current()->pcb;
    //Debug::printf("eax: %d\n", eax);

    switch(eax) {
        case 0: {
            // void exit(int status)
            SYS::exit(process, user_stack[1]);
            break;
        } case 1: {
            // ssize_t write(int fd, void* buf, size_t nbyte)
            eax = SYS::write(process, user_stack[1], (char*) user_stack[2], user_stack[3]);
            break;
        } case 2: {
            // int fork() returns {0 => child, +ve => parent, -ve => error 
            eax = SYS::fork(process, frame[0], frame[3]);
            break;
        } case 3: {
            // int sem(uint32_t init)
            eax = SYS::sem(process, user_stack[1]);
            break;
        } case 4: {
            // int up(int s)
            eax = SYS::up(process, user_stack[1]);
            break;
        } case 5: {
            // int down(int s)
            eax = SYS::down(process, user_stack[1]);
            break;
        } case 6: {
            // int close(int id)
            eax = SYS::close(process, user_stack[1]);
            break;
        } case 7: {
            // int shutdown(void)
            eax = SYS::shutdown();
            break;
        } case 8: {
            // int wait(int id, uint32_t *ptr)
            eax = SYS::wait(process, user_stack[1], (uint32_t*) user_stack[2]);
            break;
        } case 9: {
            // int execl(const char* path, const char* arg0, ....);
            eax = SYS::save_args_call_exec(user_stack);
            break;
        } case 10: {
            // int open(const char* fn)
            eax = SYS::open(process, (char*) user_stack[1], user_stack[2]);
            break;
        } case 11: {
            // ssize_t len(int fd)
            eax = SYS::len(process, user_stack[1]);
            break;
        } case 12: {
            // ssize_t read(int fd, void* buffer, size_t n)
            eax = SYS::read(process, user_stack[1], (void*) user_stack[2], user_stack[3]);
            break;
        } case 13: {
            // off_t seek(int fd, off_t off)
            eax = SYS::seek(process, user_stack[1], user_stack[2]);
            break;
        } default: {
            eax = -1;
            break;
        }       
    }
    return eax;
    
}