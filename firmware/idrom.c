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

...

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

    *(uint32_t*)(&hm2_register_file[0x0100]) = 0x55aacafe;
    hm2_register_file[0x0104] = 'H';
    hm2_register_file[0x0105] = 'O';
    hm2_register_file[0x0106] = 'S';
    hm2_register_file[0x0107] = 'T';
    hm2_register_file[0x0108] = 'M';
    hm2_register_file[0x0109] = 'O';
    hm2_register_file[0x010a] = 'T';
    hm2_register_file[0x010b] = '2';
    *(uint32_t*)(&hm2_register_file[0x010c]) = 0x0400;


    //
    // "IDROM" has a bunch of high-level metadata about the board,
    // and pointers to the Module Descriptors and Pin Descriptors.
    //

    *(uint32_t*)(&hm2_register_file[0x0400]) = 2;        // IDROM type
    *(uint32_t*)(&hm2_register_file[0x0404]) = 0x0040;   // offset to Module Descriptors
    *(uint32_t*)(&hm2_register_file[0x0408]) = 0x0200;   // offset to Pin Descriptors

    hm2_register_file[0x040c] = '*';
    hm2_register_file[0x040d] = 'R';
    hm2_register_file[0x040e] = 'P';
    hm2_register_file[0x040f] = '2';

    hm2_register_file[0x0410] = '0';
    hm2_register_file[0x0411] = '4';
    hm2_register_file[0x0412] = '0';
    hm2_register_file[0x0413] = '*';

    *(uint32_t*)(&hm2_register_file[0x0414]) = 0;   // size of the "fpga"
    *(uint32_t*)(&hm2_register_file[0x0418]) = 56;  // number of pins on the "fpga"
    *(uint32_t*)(&hm2_register_file[0x041c]) = 1;   // number of ioports
    *(uint32_t*)(&hm2_register_file[0x0420]) = 20;  // total number of pins
    *(uint32_t*)(&hm2_register_file[0x0424]) = 20;  // number of pins per ioport
    *(uint32_t*)(&hm2_register_file[0x0428]) = 10*1000*1000;  // ClockLow
    *(uint32_t*)(&hm2_register_file[0x042c]) = 20*1000*1000;  // ClockHigh
    *(uint32_t*)(&hm2_register_file[0x0430]) = 4;    // Instance Stride 0
    *(uint32_t*)(&hm2_register_file[0x0434]) = 64;   // Instance Stride 1
    *(uint32_t*)(&hm2_register_file[0x0438]) = 256;  // Register Stride 0
    *(uint32_t*)(&hm2_register_file[0x043c]) = 256;  // Register Stride 1


    //
    // "Module Descriptors" describe the firmware features present in
    // this firmware.
    //
    // 0x440.. Module descriptions 0 through 31
    // Each module descriptor is three doublewords with the following record structure:
    //
    // 0x440: (from least to most significant order)
    // GTag(0          (byte) = General function tag
    // Version(0)      (byte) = module version
    // ClockTag(0)     (byte) = Whether module uses ClockHigh or ClockLow
    // Instances(0)    (byte) = Number of instances of module in configuration
    // BaseAddress(0)  (word) = offset to module. This is also specific register = Tag
    // Registers(0)    (byte) = Number of registers per module
    // Strides(0)      (byte) = Specifies which strides to use
    // MPBitmap(0)     (Double) = bit map of which registers are multiple
    //                 '1' = multiple, LSb = reg(0)
    //
    // 0x44C: (from least to most significant order)
    // GTag(1)         (byte) = General function tag
    // Version(1)      (byte) = module version
    // ClockTag(1)     (byte) = Whether module uses ClockHigh or ClockLow
    // Instances(1)    (byte) = Number of instances of module in configuration
    // BaseAddress(1)  (word) = offset to module. This is also specific register = Tag
    // Registers(1)    (byte) = Number of registers per module
    // Strides(1)      (byte) = Specifies which strides to use
    // MPBitmap(1)     (Double) = bit map of which registers are multiple
    //                 '1' = multiple, LSb = reg(0)
    //
    // The `Strides` byte specifies which of the two available
    // InstanceStrides and RegisterStrides this module uses.  The bits
    // are:
    //     MSB         LSB
    //     x x I I x x R R
    //
    //     II == 0 for InstanceStride0
    //     II == 1 for InstanceStride1
    //     RR == 0 for RegisterStride0
    //     RR == 1 for RegisterStride1
    //

    hm2_register_file[0x0440] = HM2_GTAG_IOPORT;  // gtag
    hm2_register_file[0x0441] = 0;                // version
    hm2_register_file[0x0442] = 1;                // which clock to use
    hm2_register_file[0x0443] = 1;                // number of instances

    hm2_register_file[0x0444] = 0x00;             // base address
    hm2_register_file[0x0445] = 0x10;             //

    hm2_register_file[0x0446] = 5;                // number of registers
    hm2_register_file[0x0447] = 0x00;             // use InstanceStride0 (4) and RegisterStride0 (256)

    hm2_register_file[0x0448] = 0x1f;             // bitmap of which registers are per-channel
    hm2_register_file[0x0449] = 0x00;             //
    hm2_register_file[0x044a] = 0x00;             //
    hm2_register_file[0x044b] = 0x00;             //

    hm2_register_file[0x044c] = HM2_GTAG_END;     // gtag


    //
    // "Pin Descriptors" describe how the pins on this board are used
    // by the firmware modules.
    //
    // 0x600 Pin Descriptors
    //
    // This IO region contains the Pin Descriptors, starting at 0 and going
    // up to (IDROM.IOWidth-1) or 143, whichever is less.  (144 is the max
    // number of IO pins currently supported by HostMot2.)  Unlike the Module
    // Descriptor array (described above), there is no sentinel at the end
    // of the PD array, instead the array size is determined by the IDRom.
    //
    // There is one Pin Descriptor for each I/O pin.
    // Each pin descriptor is a doubleword with the following record structure:
    //
    // 0x600: (from least to most significant order)
    // SecPin(0)       (byte) = Which pin of secondary function connects here
    //                 eg: A,B,IDX.
    //                 Output pins have bit 7 = '1'
    // SecTag(0)       (byte) = Secondary function type (PWM,QCTR etc).
    //                 Same as module GTag
    // SecUnit(0)      (byte) = Which secondary unit or channel connects here.
    //                 If bit 7 is set, the pin is shared by all secondary units.
    // PrimaryTag(0)   (byte) = Primary function tag (normally I/O port)
    //
    // 0x604:(from least to most significant order)
    // SecPin(1)       (byte) = Which pin of secondary function connects here
    //                 eg: A,B,IDX.
    //                 Output pins have bit 7 = '1'
    // SecTag(1)       (byte) = Secondary function type (PWM,QCTR etc).
    //                 Same as module GTag
    // SecUnit(1)      (byte) = Which secondary unit or channel connects here
    // PrimaryTag(1)   (byte) = Primary function tag (normally I/O port)

#define PIN_DESCRIPTOR(secondary_pin, secondary_tag, secondary_unit, primary_tag) (uint32_t)((primary_tag << 24) | (secondary_unit << 16) | (secondary_tag << 8) | (secondary_pin))

    uint32_t * pd_reg = (uint32_t *)&hm2_register_file[0x0600];

    pd_reg[0] =  PIN_DESCRIPTOR(0x81, HM2_GTAG_STEPGEN, 0x00, HM2_GTAG_IOPORT);
    pd_reg[1] =  PIN_DESCRIPTOR(0x82, HM2_GTAG_STEPGEN, 0x00, HM2_GTAG_IOPORT);
    pd_reg[2] =  PIN_DESCRIPTOR(0x81, HM2_GTAG_STEPGEN, 0x01, HM2_GTAG_IOPORT);
    pd_reg[3] =  PIN_DESCRIPTOR(0x82, HM2_GTAG_STEPGEN, 0x01, HM2_GTAG_IOPORT);
    pd_reg[4] =  PIN_DESCRIPTOR(0x81, HM2_GTAG_STEPGEN, 0x02, HM2_GTAG_IOPORT);
    pd_reg[5] =  PIN_DESCRIPTOR(0x82, HM2_GTAG_STEPGEN, 0x02, HM2_GTAG_IOPORT);
    pd_reg[6] =  PIN_DESCRIPTOR(0x81, HM2_GTAG_STEPGEN, 0x03, HM2_GTAG_IOPORT);
    pd_reg[7] =  PIN_DESCRIPTOR(0x82, HM2_GTAG_STEPGEN, 0x03, HM2_GTAG_IOPORT);
    pd_reg[8] =  PIN_DESCRIPTOR(0x81, HM2_GTAG_STEPGEN, 0x04, HM2_GTAG_IOPORT);
    pd_reg[9] =  PIN_DESCRIPTOR(0x82, HM2_GTAG_STEPGEN, 0x04, HM2_GTAG_IOPORT);
    pd_reg[10] = PIN_DESCRIPTOR(0x81, HM2_GTAG_STEPGEN, 0x05, HM2_GTAG_IOPORT);
    pd_reg[11] = PIN_DESCRIPTOR(0x82, HM2_GTAG_STEPGEN, 0x05, HM2_GTAG_IOPORT);
    pd_reg[12] = PIN_DESCRIPTOR(0x81, HM2_GTAG_STEPGEN, 0x06, HM2_GTAG_IOPORT);
    pd_reg[13] = PIN_DESCRIPTOR(0x82, HM2_GTAG_STEPGEN, 0x06, HM2_GTAG_IOPORT);
    pd_reg[14] = PIN_DESCRIPTOR(0x81, HM2_GTAG_STEPGEN, 0x07, HM2_GTAG_IOPORT);
    pd_reg[15] = PIN_DESCRIPTOR(0x82, HM2_GTAG_STEPGEN, 0x07, HM2_GTAG_IOPORT);
    pd_reg[16] = PIN_DESCRIPTOR(0x81, HM2_GTAG_STEPGEN, 0x08, HM2_GTAG_IOPORT);
    pd_reg[17] = PIN_DESCRIPTOR(0x82, HM2_GTAG_STEPGEN, 0x08, HM2_GTAG_IOPORT);
    pd_reg[18] = PIN_DESCRIPTOR(0x81, HM2_GTAG_STEPGEN, 0x09, HM2_GTAG_IOPORT);
    pd_reg[19] = PIN_DESCRIPTOR(0x82, HM2_GTAG_STEPGEN, 0x09, HM2_GTAG_IOPORT);


    return 0;
}
