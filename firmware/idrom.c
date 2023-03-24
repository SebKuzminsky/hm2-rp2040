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


int idrom_init(void) {
    //
    // "ID" is just the cookie (0x55aacafe) and the firmware name
    // ("HOSTMOT2"), at 0x0100.
    //

    uint8_t * id_reg = hm2_fw_register("id", 0x0100, 16, NULL);
    if (id_reg == NULL) {
        return -1;
    }

    ((uint32_t*)id_reg)[0] = 0x55aacafe;
    id_reg[4]  = 'H';
    id_reg[5]  = 'O';
    id_reg[6]  = 'S';
    id_reg[7]  = 'T';
    id_reg[8]  = 'M';
    id_reg[9]  = 'O';
    id_reg[10] = 'T';
    id_reg[11] = '2';
    ((uint32_t*)id_reg)[3] = 0x0400;


    //
    // "IDROM" has a bunch of high-level metadata about the board,
    // and pointers to the Module Descriptors and Pin Descriptors.
    //

    uint8_t * idrom_reg = hm2_fw_register("idrom", 0x0400, 0x40, NULL);
    if (idrom_reg == NULL) {
        return -1;
    }

    ((uint32_t*)idrom_reg)[0] = 2;     // IDROM type
    ((uint32_t*)idrom_reg)[1] = 0x0040;   // offset to Module Descriptors
    ((uint32_t*)idrom_reg)[2] = 0x0200;   // offset to Pin Descriptors

    idrom_reg[12] = '*';
    idrom_reg[13] = 'R';
    idrom_reg[14] = 'P';
    idrom_reg[15] = '2';

    idrom_reg[16] = '0';
    idrom_reg[17] = '4';
    idrom_reg[18] = '0';
    idrom_reg[19] = '*';

    ((uint32_t*)idrom_reg)[5] = 0;   // size of the "fpga"
    ((uint32_t*)idrom_reg)[6] = 56;  // number of pins on the "fpga"
    ((uint32_t*)idrom_reg)[7] = 1;   // number of ioports
    ((uint32_t*)idrom_reg)[8] = 20;  // total number of pins
    ((uint32_t*)idrom_reg)[9] = 20;  // number of pins per ioport
    ((uint32_t*)idrom_reg)[10] = 10*1000*1000;  // ClockLow
    ((uint32_t*)idrom_reg)[11] = 20*1000*1000;  // ClockHigh
    ((uint32_t*)idrom_reg)[12] = 10;  // Instance Stride 0
    ((uint32_t*)idrom_reg)[13] = 20;  // Instance Stride 1
    ((uint32_t*)idrom_reg)[14] = 30;  // Register Stride 0
    ((uint32_t*)idrom_reg)[15] = 40;  // Register Stride 1


    return 0;
}
