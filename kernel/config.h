#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "stdint.h"

struct ApicInfo {
    uint8_t processorId;
    uint8_t apicId;
    uint32_t flags;
} __attribute__((packed));

struct RedirectionEntry {
    uint32_t lower;
    uint32_t upper;

    uint8_t vector() {
        return lower & 0xFF; // bit[0-7]
    }

    void setVector(uint8_t vector) {
        lower = ((lower & 0xFFFFFF00) | (vector));
    }

    uint8_t delvMode() {
        return (lower & 0x700) >> 8; // bits[8-10]
    }

    void setDelvMode(uint8_t delvMode) {
        lower = ((lower & 0xFFF8FF) | (delvMode << 8));
    }

    uint8_t destMode() {
        return (lower & 0x800) >> 8; // bits[11]
    }

    uint8_t delvStat() {
        return (lower & 0x1000) >> 12; // bits[12]
    }

    uint8_t pinPolarity() {
        return (lower & 0x2000) >> 12; // bits[13]
    }

    uint8_t remoteIRR() {
        return (lower & 0x4000) >> 12; // bits[14]
    }

    uint8_t triggerMode() {
        return (lower & 0x8000) >> 12; // bits[15]
    }

    uint8_t mask() {
        return (lower & 0x10000) >> 16; // bits[16]
    }

    void setMask(uint8_t mask) {
        mask = mask & 0x1;
        lower = ((lower & 0xFEFFFF) | (mask << 16));
    }

    uint8_t destination() {
        return (upper & 0xFF000000) >> 24;
    }

    void print();
};

typedef struct ApicInfo ApicInfo;

#define MAX_PROCS 16

struct Config {
    uint32_t memSize;
    uint32_t nOtherProcs;
    uint32_t totalProcs;
    uint32_t localAPIC;
    uint32_t madtFlags;
    uint32_t ioAPIC;

    ApicInfo apicInfo[MAX_PROCS];
    char oemid[7];

    static uint32_t readIOApic(uint32_t reg);

    static void writeIOApic(uint32_t reg, uint32_t value);

    static void printIOAPICEntires();
};

typedef struct Config Config;
extern Config kConfig;

extern void configInit(Config* config);


RedirectionEntry getRedirectionEntry(uint32_t reg);
void writeRedirectionEntry(uint32_t reg, RedirectionEntry newRE);

#endif
