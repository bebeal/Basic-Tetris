#ifndef _SYS_H_
#define _SYS_H_

#include "stdint.h"
#include "shared.h"
#include "processes.h"

class SYS {
public:
    static void init(void);
    static void exit(Shared<gheith::PCB> process, int rc);
    static int open(Shared<gheith::PCB> process, const char* fn, int flags);
    static ssize_t len(Shared<gheith::PCB> process, int fd);
    static ssize_t write(Shared<gheith::PCB> process, int fd, void* buf, size_t nbyte);
    static ssize_t read(Shared<gheith::PCB> process, int fd, void* buf, size_t nbyte);
    static int sem(Shared<gheith::PCB> process, uint32_t count);
    static int up(Shared<gheith::PCB> process, int sd);
    static int down(Shared<gheith::PCB> process, int sd);
    static int close(Shared<gheith::PCB> process, int id);
    static int shutdown(void);
    static int wait(Shared<gheith::PCB> process, int cd, uint32_t *status);
    static off_t seek(Shared<gheith::PCB> process, int fd, off_t offset);
    static int fork(Shared<gheith::PCB> process, uint32_t pc, uint32_t esp);
    static int exec(const char* path, int argc, const char** argv, bool delete_args);
    static int exec(const char* path, int argc, const char** argv);
    static int save_args_call_exec(uint32_t* user_stack);
    static void setup_exec_stack(int argc, const char** argv, uint32_t stack_start, uint32_t bytes_needed, uint32_t* arg_lens, uint32_t* pd);
    static void delete_args(const char* path, uint32_t argc, const char** argv);
};

#endif
