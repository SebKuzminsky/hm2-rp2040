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
    reg = hm2_fw_register("idrom", 0x0100, 16, NULL);
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
    return 0;
}
