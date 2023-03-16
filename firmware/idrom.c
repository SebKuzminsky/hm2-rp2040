#include <stdio.h>
#include "pico/stdlib.h"

#include "hm2-fw.h"


/*

From the regmap:

First ID stuff

0x0100  Config cookie   = 0x55AACAFE
0x0104  First 4 characters of configuration name
0x0108  Last 4 characters of configuration name
0x010C  Offset to IDROM location (normally 0x00000400)

LEDS

0x0200  LEDS qre in MS byte

Then the IDROM

0x0400  Normal IDROM location

0x400   IDROMType               2 for this type
0x404   OffsetToModules         64 for this type
0x408   OffsetToPindesc         512 for this type
0x40C   BoardNameLow
0x410   BoardNameHigh
0x414   FPGA size
0x418   FPGA pins
0x41C   IOPorts
0x420   IOWidth
0x424   PortWidth               Normally 24
0x428   ClockLow                In Hz   (note:on 5I20/4I65 = PCI clock
                                guessed as 33.33 MHz)
0x42C   ClockHigh               In Hz
0x430   InstanceStride0         Stride between register instances (option 0)
0x434   InstanceStride1         Stride between register instances (option 1)
0x438   RegisterStride0         Stride between different registers (option 0)
0x43C   RegisterStride1         Stride between different registers (option 1)

*/


static uint8_t * reg;


int idrom_init(void) {
    reg = hm2_fw_register("id", 0x0100, 16, NULL);
    if (reg == NULL) {
        return -1;
    }
    ((uint32_t*)reg)[0] = htonl(0x55aacafe);
    reg[4]  = 'T';
    reg[5]  = 'S';
    reg[6]  = 'O';
    reg[7]  = 'H';
    reg[8]  = '2';
    reg[9]  = 'T';
    reg[10] = 'O';
    reg[11] = 'M';
    ((uint32_t*)reg)[3] = htonl(0x0400);

    reg = hm2_fw_register("idrom", 0x0400, 0x40, NULL);
    if (reg == NULL) {
        return -1;
    }
    ((uint32_t*)reg)[0] = htonl(2);     // IDROM type
    ((uint32_t*)reg)[1] = htonl(64);    // offset to Module Descriptors
    ((uint32_t*)reg)[2] = htonl(512);   // offset to Pin Descriptors
    reg[12] = '2';
    reg[13] = 'P';
    reg[14] = 'R';
    reg[15] = '*';
    reg[16] = '*';
    reg[17] = '0';
    reg[18] = '4';
    reg[19] = '0';
    ((uint32_t*)reg)[5] = htonl(0);   // size of the "fpga"
    ((uint32_t*)reg)[6] = htonl(56);  // number of pins on the "fpga"
    ((uint32_t*)reg)[7] = htonl(1);   // number of ioports
    ((uint32_t*)reg)[8] = htonl(25);  // total number of pins
    ((uint32_t*)reg)[9] = htonl(25);  // number of pins per ioport
    ((uint32_t*)reg)[10] = htonl(10*1000*1000);  // ClockLow
    ((uint32_t*)reg)[11] = htonl(20*1000*1000);  // ClockHigh
    ((uint32_t*)reg)[12] = htonl(10);  // Instance Stride 0
    ((uint32_t*)reg)[13] = htonl(20);  // Instance Stride 1
    ((uint32_t*)reg)[14] = htonl(30);  // Register Stride 0
    ((uint32_t*)reg)[15] = htonl(40);  // Register Stride 1

    return 0;
}
