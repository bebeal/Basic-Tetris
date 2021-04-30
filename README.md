# Basic-Tetris with a minimal Linux Kernel

## Need to Implement
* Keyboard Inputs
* Writing to VGA Memory
* Tetris

## Keyboard Inputs and Enabling Keyboard Interrupts

### PS/2 Controller and Devices

### Map IRQ1 to IOAPIC

### Enabling IRQ 1
The IOAPIC has a set of 3x 32-bit registers and various 64-bit registers (one per IRQ)
* IOAPICID Register 0x00 (32-bit register) : Get/set the IO APIC's id in bits 24-27. All other bits are reserved.
* IOAPICVER Register 0x01 (32-bit register) : Get the version in bits 0-7. Get the maximum amount of redirection entries in bits 16-23. All other bits are reserved. Read only.
* IOAPICARB Register 0x03 (32-bit register) : Get the arbitration priority in bits 24-27. All other bits are reserved. Read only.
* IOREDTBL 0x10 to 0x3F (64-bit registers) : Contains a list of redirection entries. They can be read from and written to. Each entries uses two addresses, e.g. 0x12 and 0x13
  * Each Entry consists of the following
    | Field	| Bits|	Description| 
    | ----------- | ----------- | --------- |
    | Vector	| 0 - 7| 	The Interrupt vector that will be raised on the specified CPU(s).| 
    | Delivery Mode	| 8 - 10| 	How the interrupt will be sent to the CPU(s). It can be 000 (Fixed), 001 (Lowest Priority), 010 (SMI), 100 (NMI), 101 (INIT) and 111 (ExtINT). Most of the cases you want Fixed mode, or Lowest Priority if you don't want to suspend a high priority task on some important Processor/Core/Thread.| 
    | Destination Mode	| 11| 	Specify how the Destination field shall be interpreted. 0: Physical Destination, 1: Logical Destination| 
    | Delivery Status	| 12| 	If 0, the IRQ is just relaxed and waiting for something to happen (or it has fired and already processed by Local APIC(s)). If 1, it means that the IRQ has been sent to the Local APICs but | it's still waiting to be delivered.| 
    | Pin Polarity	| 13| 	0: Active high, 1: Active low. For ISA IRQs assume Active High unless otherwise specified in Interrupt Source Override descriptors of the MADT or in the MP Tables.| 
    | Remote IRR	| 14| 	TODO| 
    | Trigger Mode	| 15| 	0: Edge, 1: Level. For ISA IRQs assume Edge unless otherwise specified in Interrupt Source Override descriptors of the MADT or in the MP Tables.| 
    | Mask	| 16|	Just like in the old PIC, you can temporary disable this IRQ by setting this bit, and reenable it by clearing the bit.| 
    | Destination	| 56 - 63| 	This field is interpreted according to the Destination Format bit. If Physical destination is choosen, then this field is limited to bits 56 - 59 (only 16 CPUs addressable). You put here the APIC ID of the CPU that you want to receive the interrupt. TODO: Logical destination format...| 

The registers are memory indexed meaning the IOAPIC actually only has 2x 32-bit registers called IOREGSEL and IOREGWIN that you fetch/set and retrieve from. To fetch/set one of registers listed above you set IOREGSEL with the register index and read/write from IOREGWIN like so:
```C++
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
```

### Setting the IDT

Reference: https://wiki.osdev.org/APIC and https://wiki.osdev.org/IOAPIC


## Writing to VGA Memory


## Tetris
