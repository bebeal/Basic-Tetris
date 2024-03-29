#include "stdint.h"
#include "debug.h"
#include "config.h"

struct MemInfo {
    uint16_t ax;
    uint16_t bx;
    uint16_t cx;
    uint16_t dx;
};

extern struct MemInfo memInfo;

struct SDT {
  char Signature[4];
  uint32_t Length;
  uint8_t Revision;
  uint8_t Checksum;
  char OEMID[6];
  char OEMTableID[8];
  uint32_t OEMRevision;
  uint32_t CreatorID;
  uint32_t CreatorRevision;
} __attribute__ ((packed));

typedef struct SDT SDT;

struct MADT {
    SDT sdt;
    uint32_t localAddress;
    uint32_t flags;
} __attribute__ ((packed));

typedef struct MADT MADT;

struct MADT_ENTRY {
    uint8_t type;
    uint8_t len;
} __attribute__ ((packed));

typedef struct MADT_ENTRY MADT_ENTRY;


struct APIC_ENTRY {
    MADT_ENTRY madt;
    uint8_t processorId;
    uint8_t apicId;
    uint32_t flags;
} __attribute__ ((packed));

typedef struct APIC_ENTRY APIC_ENTRY;

struct IOAPIC_ENTRY {
    MADT_ENTRY madt;
    uint8_t apicId;
    uint8_t reserved;
    uint32_t address;
    uint32_t base;
} __attribute__ ((packed));

typedef struct IOAPIC_ENTRY IOAPIC_ENTRY;

struct RSD {
    char Signature[8];
    uint8_t Checksum;
    char OEMID[6];
    uint8_t Revision;
    SDT* rootSDT;
} __attribute__ ((packed));

typedef struct RSD RSD;

static int iseq(const char* a, const char* b, uint32_t len) {
    for (uint32_t i = 0; i<len; i++) {
        if (a[i] != b[i]) return 0;
    }
    return 1;
}

static RSD* findRSDInRange(uint32_t start, uint32_t end) {
    for (uint32_t i = start; i < end; i+= 16) {
        RSD *p = (RSD*)i;
        if (iseq(p->Signature,"RSD PTR ",8)) return p;
    }
    return 0;
}

static RSD* findRSD() {
    RSD* it = findRSDInRange(0x000E0000,0x00100000);

    if (it == 0) {
        uint32_t base = ((uint32_t) (*((uint16_t*) 0x40E))) << 4;
        it = findRSDInRange(base, base + (1 << 10));
    }
    return it;
}

static SDT* findSDT(RSD* rsdp, const char* name) {
    SDT *rsdt = rsdp->rootSDT;
    SDT **others = (SDT**) (rsdt + 1);
    int entries = (rsdt->Length - sizeof(SDT)) / 4;
 
    for (int i = 0; i < entries; i++)
    {
        SDT *h = others[i];
        if (iseq(h->Signature,name,4)) return h;
    }
 
    return 0;
}

static uint32_t memAbove1M() {
   ASSERT((memInfo.cx == 15 * 1024) || (memInfo.dx == 0));

   return (((uint32_t) (memInfo.ax | memInfo.cx)) << 10) +
          (((uint32_t) (memInfo.bx | memInfo.dx)) << 16);

}

Config kConfig;

void configInit(Config* config) {
    config->memSize = memAbove1M() + (1 << 20);
    RSD* rsdp = findRSD();
    //Debug::printf("found rsd %x\n",(uint32_t)rsdp);
    if (rsdp != 0) {
        //char oemid[7];
        for (int i=0; i<6; i++) {
           config->oemid[i] = rsdp->OEMID[i];
        }
        config->oemid[6] = 0;
        //Debug::printf("%s\n",oemid);
        //Debug::printf("RSD Revision %d\n",(uint32_t) rsdp->Revision);
    }

    MADT* madt = (MADT*) findSDT(rsdp,"APIC");
    //Debug::printf("found madt %x\n",(uint32_t) madt);

    config->localAPIC = madt->localAddress;
    config->madtFlags = madt->flags;

    int32_t bytesForEntries = madt->sdt.Length - sizeof(MADT);
    //Debug::printf("MADT bytes for entries %x\n",bytesForEntries);

    MADT_ENTRY* entryPtr = (MADT_ENTRY*)(madt + 1);

    config->nOtherProcs = 0; // Base processor not in tables

    while (bytesForEntries > 0) {
        uint32_t len = entryPtr->len;
        //Debug::printf("entry type %x\n",entryPtr->type);
        //Debug::printf("entry length %x\n",len);
        entryPtr = (MADT_ENTRY*) (((uintptr_t) entryPtr) + len);
        bytesForEntries -= len;

        if (entryPtr->type == 0) {
            APIC_ENTRY *apic = (APIC_ENTRY*) entryPtr;
            if (apic->processorId == 0) {
                continue;
            }
            ApicInfo * info = &config->apicInfo[config->nOtherProcs ++];
            info->processorId = apic->processorId;
            info->apicId = apic->apicId;
            info->flags = apic->flags;
        }
        else if (entryPtr->type == 1) {
            IOAPIC_ENTRY *apic = (IOAPIC_ENTRY*) entryPtr;
            //Debug::printf("ID: %d, address: 0x%x, base: %d\n", apic->apicId, apic->address, apic->base);
            config->ioAPIC = apic->address;
        }
    }

    config->totalProcs = config->nOtherProcs + 1;
}


uint32_t Config::readIOApic(uint32_t reg) {
   uint32_t volatile *ioapic = (uint32_t volatile *)kConfig.ioAPIC;
   ioapic[0] = (reg & 0xff); // set IOREGSEL using bottom 8 bits for register select
   return ioapic[4]; // fetch from IOREGWIN
}

void Config::writeIOApic(uint32_t reg, uint32_t value) {
   uint32_t volatile *ioapic = (uint32_t volatile *)kConfig.ioAPIC;
   ioapic[0] = (reg & 0xff); // set IOREGSEL using bottom 8 bits for register select
   ioapic[4] = value; // set IOREGWIN
}


RedirectionEntry getRedirectionEntry(uint32_t reg) {
    RedirectionEntry re;
    re.lower = Config::readIOApic(reg);
    re.upper = Config::readIOApic(reg + 1);
    return re;
}

void writeRedirectionEntry(uint32_t reg, RedirectionEntry newRE) {
    Config::writeIOApic(reg, newRE.lower);
    Config::writeIOApic(reg + 1, newRE.upper);
}

void openIRQ(uint32_t irq) {
    // the Redirection Entry register for IRQ n 0x10 + n * 2. (+ 1) for upper 32 bit values
    uint32_t reg = 0x10 + (irq * 2);
    RedirectionEntry re = getRedirectionEntry(reg);
    re.setMask(0);
    writeRedirectionEntry(reg, re);
}

void closeIRQ(uint32_t irq) {
    // the Redirection Entry register for IRQ n 0x10 + n * 2. (+ 1) for upper 32 bit values
    uint32_t reg = 0x10 + (irq * 2);
    RedirectionEntry re = getRedirectionEntry(reg);
    re.setMask(1);
    writeRedirectionEntry(reg, re);
}

void RedirectionEntry::print() {
    Debug::printf("Vector: 0x%x\n", vector());
    Debug::printf("Delivery Mode: 0x%x\n", delvMode());
    Debug::printf("Destination Mode: 0x%x\n", destMode());
    Debug::printf("Pin Polarity: 0x%x\n", pinPolarity());
    Debug::printf("Remote IRR: 0x%x\n", remoteIRR());
    Debug::printf("Trigger Mode: 0x%x\n", triggerMode());
    Debug::printf("Mask: 0x%x\n", mask());
    Debug::printf("Destination: 0x%x\n", destination());
}

void Config::printIOAPICEntires() {
    uint32_t max = (Config::readIOApic(0x1) >> 16) & 0xFF;
    for(uint32_t current = 0; current < max; current += 2) {
        RedirectionEntry re = getRedirectionEntry(current + 0x10);
        Debug::printf("\nCurrent Entry: 0x%x 0x%x\n", current + 0x10, current + 0x11);
        re.print();
    }
    Debug::printf("\n");
}