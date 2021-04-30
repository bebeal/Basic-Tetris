# Basic-Tetris with a minimal Linux Kernel

## Need to Implement
* <del>Keyboard Inputs</del>
* Writing to VGA Memory
* Tetris

## Keyboard Inputs and Enabling Keyboard Interrupts
### PS/2 Controller and Devices
The PS/2 Keyboard is a device that talks to a PS/2 controller (Intel 8042; AIP) using serial communication. The PS/2 Keyboard accepts commands and sends responses to those commands, and also sends scan codes indicating when a key was pressed or released to the PS/2 controller.

<strong>Hardware Overview: </strong>

<img src="img/Ps2-kbc.png" alt="Ps2-kbc">

##### Dual Channel PS/2 Controllers
Dual Channels PS/2 Controllers (like the one displayed above) support PS/2 style mice along with PS/2 keyboards. The first PS/2 connector (typically the keyboard) connects to PIC1 (Master PIC) thorugh IRQ1. The Second PS/2 Controller (typically the mouse) talks to PIC2 (Slave PIC) through IRQ12, which cascades to PIC1 through IRQ2.

#### PS/2 Controller IO Ports
The PS/2 Controller itself uses 2 IO ports
|IO Port |	Access Type |	Purpose |
|--|--|--|
|0x60 |	Read/Write | 	Data Port |
|0x64 |	Read |	Status Register |
|0x64 |	Write |	Command Register |

##### The Data Port
Used for reading data received from a PS/2 device or from the PS/2 controller itself writing data to a PS/2 device or the controller itself
##### Status Register
The Status Register contains various flags that show the state of the PS/2 controller. The meanings for each bit are:
|Bit |	Meaning |
|--|--|
|0 |	Output buffer status (0 = empty, 1 = full) <br> (must be set before attempting to read data from IO port 0x60) |
|1 |	Input buffer status (0 = empty, 1 = full) <br> (must be clear before attempting to write data to IO port 0x60 or IO port 0x64) |
|2 |	System Flag <br> Meant to be cleared on reset and set by firmware (via. PS/2 Controller Configuration Byte) if the system passes self tests (POST) |
|3 |	Command/data (0 = data written to input buffer is data for PS/2 device, 1 = data written to input buffer is data for PS/2 controller command) |
|4 |	Unknown (chipset specific) <br> May be "keyboard lock" (more likely unused on modern systems) |
|5 |	Unknown (chipset specific) <br> May be "receive time-out" or "second PS/2 port output buffer full"|
|6 |	Time-out error (0 = no error, 1 = time-out error) |
|7 |	Parity error (0 = no error, 1 = parity error) |
##### Command Register  
The Command Port (IO Port 0x64) is used for sending commands to the PS/2 Controller (not to PS/2 devices).

The PS/2 Controller accepts commands and performs them. These commands should not be confused with bytes sent to a PS/2 device (e.g. keyboard, mouse).

To send a command to the controller, write the command byte to IO port 0x64. If there is a "next byte" (the command is 2 bytes) then the next byte needs to be written to IO Port 0x60 after making sure that the controller is ready for it (by making sure bit 1 of the Status Register is clear). If there is a response byte, then the response byte needs to be read from IO Port 0x60 after making sure it has arrived (by making sure bit 0 of the Status Register is set).

|Command Byte |	Meaning |	Response Byte|
|---|--|--|
|0x20 |	Read "byte 0" from internal RAM	|Controller Configuration Byte (see below)|
|0x21 | to 0x3F	Read "byte N" from internal RAM (where 'N' is the command byte & 0x1F)|	Unknown (only the first byte of internal RAM has a standard purpose)|
|0x60 |	Write next byte to "byte 0" of internal RAM (Controller Configuration Byte, see below)	|None|
|0x61 | to 0x7F	Write next byte to "byte N" of internal RAM (where 'N' is the command byte & 0x1F)	|None|
|0xA7 |	Disable second PS/2 port (only if 2 PS/2 ports supported)	|None|
|0xA8 |	Enable second PS/2 port (only if 2 PS/2 ports supported)	|None|
|0xA9 |	Test second PS/2 port (only if 2 PS/2 ports supported)	|0x00 test passed <br>0x01 clock line stuck low <br>0x02 clock line stuck high <br>0x03 data line stuck low <br>0x04 data line stuck high|
|0xAA |	Test PS/2 Controller	|0x55 test passed <br>0xFC  test failed|
|0xAB |	Test first PS/2 port	|0x00 test passed <br>0x01 clock line stuck low <br>0x02 clock line stuck high <br>0x03 data line stuck low <br>0x04 data line stuck high|
|0xAC |	Diagnostic dump (read all bytes of internal RAM)	|Unknown|
|0xAD |	Disable first PS/2 port	|None|
|0xAE |	Enable first PS/2 port	|None|
|0xC0 |	Read controller input port	|Unknown (none of these bits have a standard/defined purpose)|
|0xC1 |	Copy bits 0 to 3 of input port to status bits 4 to 7	|None|
|0xC2 |	Copy bits 4 to 7 of input port to status bits 4 to 7	|None|
|0xD0 |	Read Controller Output Port	|Controller Output Port (see below)|
|0xD1 |	Write next byte to Controller Output Port (see below)  <br>Note: Check if output buffer is empty first |None|
|0xD2 |	Write next byte to first PS/2 port output buffer (only if 2 PS/2 ports supported) <br> (makes it look like the byte written was received from the first PS/2 port) | None |
|0xD3 |	Write next byte to second PS/2 port output buffer (only if 2 PS/2 ports supported) <br>(makes it look like the byte written was received from the second PS/2 port) |None|
|0xD4 |	Write next byte to second PS/2 port input buffer (only if 2 PS/2 ports supported) (sends next byte to the second PS/2 port) | None|
|0xF0 to 0xFF |	Pulse output line low for 6 ms. Bits 0 to 3 are used as a mask (0 = pulse line, 1 = don't pulse line) and correspond to 4 different output lines. <br>Note: Bit 0 corresponds to the "reset" line. The other output lines don't have a standard/defined purpose. | None|
##### PS/2 Controller Configuration Byte
Commands 0x20 and 0x60 let you read and write the PS/2 Controller Configuration Byte. This configuration byte has the following format:
|Bit |	Meaning|
|--|--|
|0 |	First PS/2 port interrupt (1 = enabled, 0 = disabled)|
|1 |	Second PS/2 port interrupt (1 = enabled, 0 = disabled, only if 2 PS/2 ports supported)|
|2 |	System Flag (1 = system passed POST, 0 = your OS shouldn't be running)|
|3 |	Should be zero|
|4 |	First PS/2 port clock (1 = disabled, 0 = enabled)|
|5 |	Second PS/2 port clock (1 = disabled, 0 = enabled, only if 2 PS/2 ports supported)|
|6 |	First PS/2 port translation (1 = enabled, 0 = disabled)|
|7 |	Must be zero|
##### PS/2 Controller Output Port
Commands 0xD0 and 0xD1 let you read and write the PS/2 Controller Output Port. This output port has the following format:

|Bit|	Meaning|
|--|--|
|0|	System reset (output) <br>WARNING always set to '1'. You need to pulse the reset line (e.g. using command 0xFE), and setting this bit to '0' can lock the computer up ("reset forever").|
|1|	A20 gate (output)|
|2|	Second PS/2 port clock (output, only if 2 PS/2 ports supported)|
|3|	Second PS/2 port data (output, only if 2 PS/2 ports supported)|
|4|	Output buffer full with byte from first PS/2 port (connected to IRQ1)|
|5|	Output buffer full with byte from second PS/2 port (connected to IRQ12, only if 2 PS/2 ports supported)|
|6|	First PS/2 port clock (output)|
|7|	First PS/2 port data (output)|

##### Initializing the PS/2 Controller
* Disable Devices: Send to the PS/2 controller command 0xAD to disable the first connected device and command 0xA7 to disable the second connected device
* Flush The Output Buffer: read from the Data IO Port (0x60) until the output buffer status is clear 
* Set The Controller Configuration Byte:
  * Disable All IRQs (mask them) (Re-enable after controller initialized)
  * Read The Configuration Byte (command 0x20)
  * Disable Translation And Port Interrupts (clear bits 0, 1, and 6) (using command 0x60)
* Check If Dual Channel Controller: Test if bit 5 of the configuration byte was set, if it was clear then it can't be a "dual channel" PS/2 controller (because the second PS/2 port should be disabled)
* Test The PS/2 Controller: Send command 0xAA and poll for response. (0x55 indicates test passed)
* Perform Interface Test: Send command 0xAB to the test the first PS/2 connection and poll for response. (0x55 indicates test passed). If dual channel then send command 0xA9  to test the second PS/2 connection and poll for response. (0x00 indicates test passed)
* Enable Devices: Send command 0xAE to enable the first PS/2 connection. If dual channel then send command 0xA8 to enable the second PS/2 connection
* If using IRQs then enable interrupts by setting bit 0 and bit 1 in the configuration byte (reading with 0x20 then setting with 0x60)
* Reset Devices: All PS/2 devices support the "reset" command (a command for the device, not the controller). To send the reset send the byte 0xFF to the usable device. You should recieve two responses, the first, 0xFA, indicating the device recieved the request, and then the response from the request (0xAA indicates test passed)

##### Sending Bytes to Devices
Unfortunately, the PS/2 Controller does not support interrupt driven transmission (e.g. you can't have a queue of bytes waiting to be sent and then send each byte from inside a "transmitter empty" IRQ handler). Fortunately, little data needs to be sent to typical PS/2 devices and polling suffices.

<strong>First PS/2 Port</strong>
To send data to the first PS/2 Port:
* Set up some timer or counter to use as a time-out
* Poll bit 1 of the Status Register ("Input buffer empty/full") until it becomes clear* or until your time-out expires
* If the time-out expired, return an error
* Otherwise, write the data to the Data Port (IO port 0x60)

<strong>Second PS/2 Port</strong>
Sending data to the second PS/2 port is a little more complicated, as you need to send a command to the PS/2 controller to tell it that you want to talk to the second PS/2 port instead of the first PS/2 port. To send data to the second PS/2 Port:
* Write the command 0xD4 to IO Port 0x64
* Set up some timer or counter to use as a time-out
* Poll bit 1 of the Status Register ("Input buffer empty/full") until it becomes clear, or until your time-out expires
* If the time-out expired, return an error
* Otherwise, write the data to the Data Port (IO port 0x60)

The function below can achieve both and recieve a single response. Note: no timer or counter is used to time-out so will infinitely run if not properly handled
```C++
// next_byte == 0 if no next_byte to send
// response == nullptr if no response to recieve 
static void send_command(unsigned char command_byte, unsigned char next_byte, unsigned char* response) {
    outb(0x64, command_byte); // send command_byte to Command IO Port (0x64)
    if (next_byte) {
        poll_write(); // while((inb(0x64) & 0x2));
        outb(0x60, next_byte); // send next_byte to Data IO Port (0x60)
    }
    if (response != nullptr) {
        poll_read(); // while(!(inb(0x64) & 0x1));
        *response = inb(0x60); // responses recieved via Data IO Port (0x60)
    }
}
```

### Interrupts with PS/2 Devices
When IRQ1 occurs you just read from IO Port 0x60 (there is no need to check bit 0 in the Status Register first), send the EOI (0x20) to the interrupt controller and return from the interrupt handler. You know that the data came from the first PS/2 device connected because you received an IRQ1.

When IRQ12 occurs you read from IO Port 0x60 (there is no need to check bit 0 in the Status Register first), send the EOI (0x20) to the interrupt controller/s and return from the interrupt handler. You know that the data came from the second PS/2 device connected because you received an IRQ12.

Unfortunately, there is one problem to worry about. If you send a command to the PS/2 controller that involves a response, the PS/2 controller may generate IRQ1, IRQ12, or no IRQ (depending on the firmware) when it puts the "response byte" into the buffer. In all three cases, you can't tell if the byte came from a PS/2 device or the PS/2 controller. In the no IRQ case, you additionally will need to poll for the byte. Fortunately, you should never need to send a command to the PS/2 controller itself after initialisation (and you can disable IRQs and both PS/2 devices where necessary during initialisation).

#### The IOAPIC 
The IOAPIC has a set of 3x 32-bit registers and various 64-bit registers (one per IRQ)
* IOAPICID Register 0x00 (32-bit register) : Get/set the IO APIC's id in bits 24-27. All other bits are reserved.
* IOAPICVER Register 0x01 (32-bit register) : Get the version in bits 0-7. Get the maximum amount of redirection entries in bits 16-23. All other bits are reserved. Read only.
* IOAPICARB Register 0x03 (32-bit register) : Get the arbitration priority in bits 24-27. All other bits are reserved. Read only.
* IOREDTBL 0x10 to 0x3F (64-bit registers) : Contains a list of redirection entries. They can be read from and written to. Each entries uses two addresses, e.g. IRQ1 corresponds to 0x12 and 0x13
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

##### Reading and Writing from/to the IOAPIC

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
#### Creating a Redirection Entry in the IOAPIC
Typically the IOREDTBL is mapped to the IRQs (legacy or PCI) during startup using the APIC's MADT/APIC table. But in our mininal kernel we will need to create the entry ourselves. We simply need to create a 64-bit entry via 2x 32-bit quantities and write them to the IOAPICS 0x12 and 0x13 registers (which correspond to IRQ1, the lane our PS/2 keyboard (through the controller) communicates on). We will map the IRQ1 lane to the Interrupt Vector to 0x9 and create an entry in the IDT with this interrupt vector for the CPU later. The other entries can all stay at their default meaning when 0. Lastly we need to ensure the mask bit is cleared in order to the IOAPIC to start listening to that lane. Thus our 64-bit entry becomes:
`0000000000000000000000000000000000000000000000000000000000001001 == 0x0000000000000009`
(the first 8 bits indicate the interrupt vector the IOAPIC will send to the CPU, the rest take on their default value when 0, bit 16 the mask bit must also be 0).

Putting this entry into the IOREDTBL consists of writing the lower 32-bits into the IOAPIC register 0x12, and the upper 32-bits into 0x13, corresponding to IRQ1


### Setting the IDT
make an entry in the IDT table corresponding to the keyboard handler we wrote

Sources and References: 
* https://wiki.osdev.org/PS2_Keyboard
* https://wiki.osdev.org/APIC
* https://wiki.osdev.org/IOAPIC


## Writing to VGA Memory


## Tetris
