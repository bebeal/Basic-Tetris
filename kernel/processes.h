#ifndef _PROCESSES_H_
#define _PROCESSES_H_

#include "debug.h"
#include "threads.h"
#include "u8250.h"
#include "io.h"
#include "vmm.h"
#include "semaphore.h"
#include "future.h"
#include "ext2.h"
#include "kernel.h"
#include "atomic.h"
#include "stdint.h"

namespace gheith {
    struct TCB;
    class PCB;


    enum DescriptorType : uint32_t{
        NoneDT,
        FileDT,
        SemaphoreDT,
        ChildDT
    };

    struct Descriptor {

        DescriptorType type;

        Descriptor(DescriptorType type) : type(type){}
        Descriptor() : type(DescriptorType::NoneDT) {}

        virtual ~Descriptor() {
            type = DescriptorType::NoneDT;
        }

        virtual bool is_nothing() {
            return type == DescriptorType::NoneDT;
        }

        virtual bool is_file() {
            return type == DescriptorType::FileDT;
        }

        virtual bool is_semaphore() {
            return type == DescriptorType::SemaphoreDT;
        }

        virtual bool is_child() {
            return type == DescriptorType::ChildDT;
        }

        virtual Descriptor* copy() {
            return new Descriptor();
        }

        virtual int get() {return -1;}

        virtual int down() {return -1;}
        virtual int up() {return -1;}

        virtual ssize_t write(char* buf, size_t nbyte) {return -1;}
        virtual ssize_t read(char* buf, size_t nbyte) {return -1;}
        virtual ssize_t len() {return -1;}
        virtual off_t seek(off_t off) {return -1;};
    };

    // struct stdin : public Descriptor {
    //     InputStream<char>* in; // processes read in from stdin

    //     stdin() : Descriptor(DescriptorType::FileDT), in(new U8250) {}

    //     Descriptor* copy() {
    //         return new stdin();
    //     }
        

    // };

    // struct stdout : public Descriptor {
    //     OutputStream<char>* out; // processes write out to stdout
    //     //InterruptSafeLock lock{};

    //     stdout() : Descriptor(DescriptorType::FileDT), out(new U8250) {}

    //     Descriptor* copy() {
    //         return new stdout();
    //     }

    //     // virtual vprintf(const char* fmt, va_list ap) {
    //     //     if (out) {
    //     //         lock.lock();
    //     //         K::vsnprintf(*out,1000,fmt,ap);
    //     //         lock.unlock();
    //     //     }
    //     // }

    //     virtual int32_t write(char* buf, size_t nbyte) {
    //         for(uint32_t c = 0; c < nbyte; c++) {
    //             Debug::printf("%c", buf[c]);
    //         }
    //         return nbyte;
    //     }

    // };

    // struct stderr : public Descriptor {
    //     OutputStream<char>* out; 

    //     stderr() : Descriptor(DescriptorType::FileDT), out(new U8250) {}

    //     Descriptor* copy() {
    //         return new stderr();
    //     }
    //     // write -> put()
    // };

    struct FileDescriptor : public Descriptor {
        off_t offset;
        uint32_t permission;
        Shared<Node> file;

        FileDescriptor(off_t offset, uint32_t permission, Shared<Node> file) : Descriptor(DescriptorType::FileDT), offset(offset), permission(permission), file(file) {}
        FileDescriptor(Shared<Node> file) : FileDescriptor(0, 0, file) {}
        FileDescriptor() : FileDescriptor(Shared<Node>{}) {}

        virtual Descriptor* copy() override {
            return new FileDescriptor(offset, permission, file);
        } 

        ~FileDescriptor() {
        }

        ssize_t write(char* buf, size_t nbyte) {
            if (buf != nullptr && (uint32_t)buf >= 0x80000000 &&  (uint32_t)buf != kConfig.ioAPIC && (uint32_t)buf != kConfig.localAPIC) {
                for(uint32_t c = 0; c < nbyte; c++) {
                    Debug::printf("%c", buf[c]);
                }
                return nbyte;
            }
            return -1;
        }

        ssize_t read(char* buf, size_t nbyte) {
            if (file != nullptr && (uint32_t)buf >= 0x80000000) {
                uint32_t bytes_read = file->read_all(offset, nbyte, buf);
                offset += bytes_read;
                if (bytes_read < 0) {
                    offset -= bytes_read;
                    return -1;
                }
                return bytes_read;
            }
            return -1;
        }

        ssize_t len() {
            return file != nullptr ? file->size_in_bytes() : -1;
        }

        off_t seek(off_t off) {
            if (file == nullptr) {
                return -1;
            }
            offset = off;
            return offset;
        }
    };

    struct SemaphoreDescriptor : public Descriptor {
        Shared<Semaphore> s;

        SemaphoreDescriptor(Shared<Semaphore> s) : Descriptor(DescriptorType::SemaphoreDT), s(s) {}

        Descriptor* copy() override {
            return new SemaphoreDescriptor(s);
        }

        ~SemaphoreDescriptor() {
        }

        int down() override {
            s->down();
            return 0;
        }

        int up() override {
            s->up();
            return 0;
        }
    };

    class PCB {
    private:
        // meta info
        uint32_t pid;
        // Processes thread/s
        TCB* thread;
        // files, child processes and semaphores kept in descriptor table dt
        static const uint32_t NUM_DESCRIPTORS = 10;
        static const uint32_t NUM_DESCRIPTOR_TYPES = 3;
        Descriptor** dt; // [0:10] reserved for FileDescriptors, [10:20] reserved for SemaphoreDescriptors, [20:30] reserved for ChildDescriptors
        // number of descriptors for each type of descriptors, index == (enum class) DescriptorType - 1 
        uint32_t* count;
        Shared<Future<int>> status;
    public:
        Atomic<uint32_t> ref_count;

        // dti_f = 3 cause we start out w stdin, stdout, stderror
        PCB(uint32_t pid, TCB* thread) : pid(pid), thread(thread), dt(new Descriptor*[NUM_DESCRIPTORS * NUM_DESCRIPTOR_TYPES]), count(new uint32_t[NUM_DESCRIPTOR_TYPES]), status(Shared<Future<int>>::make()), ref_count(0) {
            // set spots for stdin, stdout, stderror
            add_descriptor(new FileDescriptor());
            add_descriptor(new FileDescriptor());
            add_descriptor(new FileDescriptor());
            for(uint32_t dti = 3; dti < NUM_DESCRIPTORS * NUM_DESCRIPTOR_TYPES; dti++) {
                dt[dti] = new Descriptor();
            }
        }

        PCB() : PCB(0, nullptr) {}

        PCB(const PCB&) = delete;

        ~PCB() {
            for(uint32_t dti = 0; dti < NUM_DESCRIPTORS * NUM_DESCRIPTOR_TYPES; dti++) {
                delete[] dt[dti];
            }
            delete[] dt;
            delete[] count;
        }

        uint32_t* get_pd();
        TCB* get_thread();
        int sem(uint32_t count);
        void copy_from(Shared<PCB> from);
        int add_child(Shared<PCB> child);
        void remove_user_space();

        uint32_t get_pid() {
            return pid;
        }

        uint32_t num_descriptors(DescriptorType type) {
            return count[type - 1];
        }

        uint32_t num_children() {
            return num_descriptors(DescriptorType::ChildDT);
        }

        uint32_t num_files() {
            return num_descriptors(DescriptorType::FileDT);
        }

        uint32_t num_semaphores() {
            return num_descriptors(DescriptorType::SemaphoreDT);
        }

        void exit(uint32_t rc) {
            status->set(rc);
        }

        uint32_t next_dti(DescriptorType type) {
            // - 1 to account for NoneDT type
            uint32_t type_index = type - 1;
            uint32_t dti = count[type_index];
            if (dti >= NUM_DESCRIPTORS) {
                return -1;
            }
            count[type_index]++;
            dti += (NUM_DESCRIPTORS * type_index); // actual index in array
            return dti;
        }

        // get the next available spot in the descriptor table
        uint32_t next_dti(Descriptor* d) {
            return next_dti(d->type);
        }

        uint32_t reserve_child_descriptor() {
            return next_dti(DescriptorType::ChildDT);
        }

        uint32_t add_descriptor(Descriptor* d) {
            uint32_t dti = next_dti(d);
            if (dti >= 0) {
                dt[dti] = d;
            } else {
                delete d;
            }
            return dti;
        }

        int open(const char* fn) {
            Shared<Node> node = bebeal::root_fs->find(bebeal::root_fs->root, fn);
            if (node == nullptr) {
                return -1;
            }
            FileDescriptor* d = new FileDescriptor(node);
            return add_descriptor(d);
        }

        int wait(uint32_t dti) {
            return get_descriptor(dti)->get();
        }

        uint32_t close(int dti) { //remove_descriptor()
            Descriptor* d = get_descriptor(dti);
            uint32_t type_index = d->type - 1;
            uint32_t count_d = dti - (NUM_DESCRIPTORS * type_index); 
            if (!d->is_nothing() && count_d < count[type_index]) {
                delete dt[dti];
                dt[dti] = new Descriptor();
                count[type_index]--;
                return 0;
            } 
            return -1;
        }

        void set_descriptor(uint32_t dti, Descriptor* d) {
            dt[dti] = d;
        }

        Descriptor* get_descriptor(uint32_t dti) {
            bool safe = dti < NUM_DESCRIPTORS * NUM_DESCRIPTOR_TYPES;
            if (safe) {
                return dt[dti];
            }
            return new Descriptor();
        }

        ssize_t len(uint32_t dti) {
            return get_descriptor(dti)->len();
        }

        ssize_t read(uint32_t dti, void* buffer, size_t nbyte) {
            return get_descriptor(dti)->read((char*) buffer, nbyte);
        }

        off_t seek(int dti, uint32_t off) {
            return get_descriptor(dti)->seek(off);
        }

        ssize_t write(int dti, char* buf, size_t nbyte) {
            return get_descriptor(dti)->write(buf, nbyte);
        }

        int up(int dti) {
            return get_descriptor(dti)->up();
        }

        int down(int dti) {
            return get_descriptor(dti)->down();
        }

        friend class Shared<PCB>;
        friend class ChildProcessDescriptor;
    };

    struct ChildProcessDescriptor : public Descriptor {
        Shared<PCB> child;

        ChildProcessDescriptor(Shared<PCB> child) : Descriptor(DescriptorType::ChildDT), child(child) {}

        int get() {
            return child->status->get();
        }

        ~ChildProcessDescriptor() {
        }
    };

} // namespace gheith 

#endif