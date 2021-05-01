/*
Reference: https://wiki.osdev.org/%228042%22_PS/2_Controller
*/

//#define DEBUG
#ifdef DEBUG
  #define D if(1)
#else 
  #define D if(0)
#endif

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "stdint.h"
#include "config.h"
#include "atomic.h"
#include "machine.h"
#include "idt.h"
#include "io.h"
#include "smp.h"
#include "debug.h"

namespace PS2 {
    constexpr uint32_t DATA_PORT = 0x60;
    constexpr uint32_t STAT_COMM_PORT = 0x64;
}

namespace SB {
    // https://wiki.osdev.org/PS/2_Keyboard Special Bytes
    constexpr char Err0 = 0x00;
    constexpr char Err2 = 0xFF;
    constexpr char ACK = 0xF4;
    constexpr char Resend = 0xFE;
    constexpr char Echo = 0xEE;
    constexpr char Pass = 0xAA;
    constexpr char Fail0 = 0xFC;
    constexpr char Fail1 = 0xFD;
}

class U8042 {
public:
    bool dual;
    constexpr static uint32_t KBVector = 0x09; 

    U8042() {
        Debug::printf("| Initializing U8042 PS/2 Port device/s\n");

        /*
        Disable Devices
        */
        D Debug::printf("| U8042: Disabling Device/s\n");
        outb(PS2::STAT_COMM_PORT, 0xAD);
        outb(PS2::STAT_COMM_PORT, 0xA7);

        /*
        Flush the output buffer
        */
        D Debug::printf("| U8042: Flushing the output buffer\n");
        while((inb(PS2::STAT_COMM_PORT) & 0x1)) {
            inb(PS2::DATA_PORT);
        }

        /*
        Set the Controller Configuration Byte
        */
        D Debug::printf("| U8042: Setting the Controller Configuration\n");
        // temporarily disable all IRQ's
        closeIRQ(1); // open this IRQ to recieve keyboard interrupts again
        unsigned char response;
        send_command(0x20, 0, &response);
        dual = (response & 0x20) != 0; // test if bit 5 was set. If it was clear, then you know it can't be a "dual channel" PS/2 controller
        D Debug::printf("| U8042: Dual Channel: %d\n", dual);
        D Debug::printf("| U8042: Controller Configuration Before: 0x%x\n", response);
        response = (response & 0xBC); // clear bits 0, 1, 6
        D Debug::printf("| U8042: Controller Configuration After: 0x%x\n", response);
        send_command(0x60, response, nullptr);

        /*
        Perform Controller Self Test
        */
        D Debug::printf("| U8042: Performing Controller Self Test\n");
        send_command(0xAA, 0, &response); // 0x55 test passed; 0xFC test failed
        if (response == 0xFC) {
            D Debug::PANIC("| U8042: Controller Test failed\n");
        } else {
            D Debug::printf("| U8042: Test Passed; Returned: 0x%x\n", response); 
        }

        /*
        Perform Interface Test
        */
        send_command(0xAB, 0, &response); // 0x00 test passed ;
        if (response != 0) {
            D Debug::printf("| U8042: First PS/2 port Test Failed; Returned: 0x%x\n", response);
        } else {
            D Debug::printf("| U8042: First PS/2 port Test Passed\n");
        }
        if (dual) {
            send_command(0xA9, 0, &response); // 0x00 test passed ;
            if (response != 0) {
                D Debug::printf("| U8042: Second PS/2 port Test Failed; Returned: 0x%x\n", response);
            } else {
                D Debug::printf("| U8042: Second PS/2 port Test Passed\n");
            }
        }

        /*
        Enable Ports
        */
        D Debug::printf("| U8042: Enabling Device/s\n");
        outb(PS2::STAT_COMM_PORT, 0xAE);
        if (dual) {
            outb(PS2::STAT_COMM_PORT, 0xA8);
        }
        send_command(0x20, 0, &response);
        // enabled interrupts for usable PS/2 ports
        D Debug::printf("| U8042: Changing Config; Old Config: 0x%x\n", response);
        response = (response | 0x41); // bit 0 for first PS/2 port interrupt, bit 6 for PS/2 port translation
        if (dual) {
            response = (response | 0x2); // bit 1 for second PS/2 port interrupt
        }
        D Debug::printf("| U8042: Changing Config; New Config: 0x%x\n", response);
        send_command(0x60, response, nullptr);

        /*
        Reset PS/2 Devices
        */
        D Debug::printf("| U8042: Resetting Device/s\n");

        // device 1
        poll_write();
        outb(PS2::DATA_PORT, 0xFF);
        poll_read();
        response = inb(PS2::DATA_PORT) & 0xFF;
        if (response == 0xFA) {
            D Debug::printf("| U8042: Device 1 Command Acknowledged: 0x%x\n", response);
        } else {
            D Debug::printf("| U8042: Device 1 responded: 0x%x\n", response);
        }
    
        poll_read();
        response = inb(PS2::DATA_PORT) & 0xFF;
        if (response == 0xFC || response == 0xFD) {
            D Debug::PANIC("| U8042: Reset Device 1 failed\n");
        } else {
            D Debug::printf("| U8042: Reset Device 1 Passed; Returned: 0x%x\n", response);
        }

        // device 2
        send_command(0xD4, 0xFF, &response);
        response = response & 0xFF;
        if (response == 0xFA) {
            D Debug::printf("| U8042: Device 2 Command Acknowledged: 0x%x\n", response);
        } else {
            D Debug::printf("| U8042: Device 2 responded: 0x%x\n", response);
        }
        poll_read();
        response = inb(PS2::DATA_PORT) & 0xFF;
        if (response == 0xFC || response == 0xFD) {
            D Debug::PANIC("| U8042: Reset Device 2 failed\n");
        } else {
            D Debug::printf("| U8042: Reset Device 2 Passed; Returned: 0x%x\n", response);
        }
        // Enable Scanning Mode to send scancodes
        enable_scanning();
    }

    /*
    make sure that the controller is ready for data (by making sure bit 1 of the Status Register is clear).
    */
    static void poll_write() {
        while((inb(PS2::STAT_COMM_PORT) & 0x2));
    }


    /*
    make sure that the controller is ready for you to read data (by making sure bit 0 of status register is set).
    */
    static void poll_read() {
        while(!(inb(PS2::STAT_COMM_PORT) & 0x1));
    }

    /*
    Will send a command to controller 
    writes command byte to PS2::STAT_COMM_PORT (0x64)
    if there is a "next_byte" then it's written to PS2::DATA_PORT (0x60) after making sure that
    controller is ready by making sure bit 1 of the Status Register is clear
    If there is a "response_byte", then it's read from PS2::DATA_PORT (0x60) after making sure it 
    has arrived (by making sure bit 0 of the Status Register is set)
    */
    static void send_command(unsigned char command_byte, unsigned char next_byte, unsigned char* response) {
        outb(PS2::STAT_COMM_PORT, command_byte);
        if (next_byte) {
            poll_write();
            outb(PS2::DATA_PORT, next_byte);
        }
        if (response != nullptr) {
            poll_read();
            *response = inb(PS2::DATA_PORT);
        }
    }

    /*
    Enables scanning
    */
    static void enable_scanning() {
        D Debug::printf("| U8042: Enabling scanning\n");
        poll_write();
        outb(PS2::DATA_PORT, 0xF4);
        poll_read();
        unsigned char response = inb(PS2::DATA_PORT);
        D Debug::printf("| U8042: Response: 0x%x\n", response);
    }

    /*
    Makes it look like char c written was received from the first PS/2 port
    */
    static void fake_keyboard_input(char c) {
        poll_write();
        send_command(0xD2, c, nullptr);
        poll_write();
        send_command(0xD2, 0xFA, nullptr); 
    }

    // random commands to devices for testing
    static void random_commands() {
        // talking to device 1
        Debug::printf("getting scan code set\n");
        poll_write();
        outb(PS2::DATA_PORT, 0xF0); // scan code set 
        poll_read();
        unsigned char response = inb(PS2::DATA_PORT);
        Debug::printf("should be 0xfa Response: 0x%x\n", response);
        poll_write();
        outb(PS2::DATA_PORT, 0); // sub command?
        poll_read();
        response = inb(PS2::DATA_PORT);
        Debug::printf("should be 0xfa Response: 0x%x\n", response);
        poll_read();
        response = inb(PS2::DATA_PORT);
        // should get getting scan code set
        Debug::printf("should be scan code response Response: 0x%x\n", response);

        Debug::printf("Getting device 1 identity\n");
        poll_write();
        outb(PS2::DATA_PORT, 0xF2);
        poll_read();
        response = inb(PS2::DATA_PORT);
        Debug::printf("should be 0xfa Response: 0x%x\n", response);
        poll_read();
        response = inb(PS2::DATA_PORT);
        Debug::printf("id: 0x%x\n", response); // 0xAB
        poll_read();
        response = inb(PS2::DATA_PORT);
        Debug::printf("id: 0x%x\n", response); // 0x41
        // 0xAB, 0x41 is MF2 keyboard with translation enabled in the PS/Controller https://wiki.osdev.org/%228042%22_PS/2_Controller#Detecting_PS.2F2_Device_Types
        
        poll_read();
        response = inb(PS2::DATA_PORT);
        Debug::printf("id: 0x%x\n", response); // where tf did this 0 come from

        // device 2
        Debug::printf("Getting device 2 identity\n");
        send_command(0xD4, 0xF2, &response);
        Debug::printf("should be 0xfa Response: 0x%x\n", response);
        poll_read();
        response = inb(PS2::DATA_PORT);
        Debug::printf("id: 0x%x\n", response); // 0x00 is Standard PS/2 mouse
    }
};


class keyboard {
    static U8042* ps2C;
public:
    static void init(U8042* ps2C);
};


#endif
