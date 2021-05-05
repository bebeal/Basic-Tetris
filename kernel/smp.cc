#include "smp.h"
#include "machine.h"
#include "debug.h"
#include "idt.h"

AtomicPtr<uint32_t> SMP::id;
AtomicPtr<uint32_t> SMP::eoi_reg;
AtomicPtr<uint32_t> SMP::spurious;
AtomicPtr<uint32_t> SMP::icr_low;
AtomicPtr<uint32_t> SMP::icr_high;
AtomicPtr<uint32_t> SMP::apit_lvt_timer;
AtomicPtr<uint32_t> SMP::apit_initial_count;
AtomicPtr<uint32_t> SMP::apit_current_count;
AtomicPtr<uint32_t> SMP::apit_divide;

Atomic<uint32_t> SMP::running {0};

const char* SMP::names[] = {
    "cpu0",
    "cpu1",
    "cpu2",
    "cpu3",
    "cpu4",
    "cpu5",
    "cpu6",
    "cpu7",
    "cpu8",
    "cpu9",
    "cpu10",
    "cpu11",
    "cpu12",
    "cpu13",
    "cpu14",
    "cpu15"
};

void SMP::init(bool isFirst) {
    if (isFirst) {
        // Define APIC registers
        id = (uint32_t *) (kConfig.localAPIC + 0x20) ;
        eoi_reg = (uint32_t *) (kConfig.localAPIC + 0xb0) ; // EOI register	write only
        spurious = (uint32_t *) (kConfig.localAPIC + 0xf0) ;
        icr_low = (uint32_t *) (kConfig.localAPIC + 0x300) ;
        icr_high = (uint32_t *) (kConfig.localAPIC + 0x310) ;
        apit_lvt_timer = (uint32_t *) (kConfig.localAPIC + 0x320);
        apit_initial_count = (uint32_t *) (kConfig.localAPIC + 0x380);
        apit_current_count = (uint32_t *) (kConfig.localAPIC + 0x390);
        apit_divide = (uint32_t *) (kConfig.localAPIC + 0x3e0);

        // Register spurious interrupt handler
        IDT::interrupt(0xff, (uint32_t) spuriousHandler_);

    }

    // disable PIC (masking all interrupts on PIC/s)
    outb(0x21,0xff);
    outb(0xa1,0xff);

    // insert IOAPIC entry for IRQ1                 
    RedirectionEntry f = getRedirectionEntry(0x12);
    f.setVector(0x9);
    f.setMask(1);
    writeRedirectionEntry(0x12, f);

    // Enable LAPIC
    uint64_t msr = rdmsr(MSR); // RDMSR: read model specific register
    wrmsr(MSR, msr | ENABLE); // WRMSR: write model specific register, ENABLE == 1 << 11 == 0x800 i.e. IA32_APIC_BASE[11] the APIC Global Enable flag (https://xem.github.io/minix86/manual/intel-x86-and-64-manual-vol3/o_fe12b1e2a880e0ce-370.html)

    spurious.set(0x1ff); // The correct value for this field is the IRQ number that you want to map the spurious interrupts to within the lowest 8 bits, and the 8th bit set, easiest is to use 0xFF


}
