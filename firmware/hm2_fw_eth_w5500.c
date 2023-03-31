/**
 * Copyright (c) 2021 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>

#include "pico/multicore.h"

#include "port_common.h"

#include "wizchip_conf.h"
#include "w5x00_spi.h"

#include "socket.h"

#include "hm2-fw.h"
#include "lbp16.h"


#define PLL_SYS_KHZ (133 * 1000)


static wiz_NetInfo g_net_info = {
    .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56}, // MAC address
    .ip = {192, 168, 1, 121},                    // IP address
    .sn = {255, 255, 255, 0},                    // Subnet Mask
    .gw = {192, 168, 1, 1},                      // Gateway
    .dns = {8, 8, 8, 8},                         // DNS server
    .dhcp = NETINFO_STATIC                       // DHCP enable/disable
};


typedef struct {
    uint16_t cookie;
    uint16_t memsizes;
    uint16_t memranges;
    uint16_t address_pointer;
    uint8_t spacename[9];  // It's really only 8 bytes, but it's convenient to store the terminating NULL.
} hm2_eth_info_area_t;


//
// MEMSIZES arguments:
//
//     writable:
//         0: read-only
//         1: writable
//
//     type:
//         01: register
//         02: memory
//         0E: EEPROM
//         0F: flash
//
//     access (bitmap):
//         1: 8 bit
//         2: 16 bit
//         4: 32 bit
//         8: 64 bit
//

#define MEMSIZES(writable, type, access) \
    ( \
        (writable << 15) \
        | (type << 8) \
        | (access) \
    )


//
// MEMRANGES arguments:
//
//     erase_block_size: Erase block size is 2^E.  Used for flash only,
//         should be 0 for non-flash memory spaces.
//
//     page_size: Page size is 2^P.  Used for flash only, should be 0
//         for non-flash memory spaces.
//
//     ps_address_range: Ps Address Range is 2^S.
//

#define MEMRANGES(erase_block_size, page_size, ps_address_range) \
    ( \
        (erase_block_size << 11) \
        | (page_size << 6) \
        | (ps_address_range) \
    )


hm2_eth_info_area_t eth_info_area[8] = {

    {
        .cookie = 0x5a00,
        .memsizes = MEMSIZES(1, 1, 4),
        .memranges = MEMRANGES(0, 0, 16),
        .address_pointer = 0x0000,
        .spacename = "HostMot2"
    },

    {
        .cookie = 0x5a01,
        .memsizes = MEMSIZES(1, 1, 2),
        .memranges = MEMRANGES(0, 0, 8),
        .address_pointer = 0x0000,
        .spacename = "W5500"
    },

    {
        .cookie = 0x5a02,
        .memsizes = MEMSIZES(1, 0x0e, 2),
        .memranges = MEMRANGES(0, 0, 7),
        .address_pointer = 0x0000,
        .spacename = "EtherEEP"
    },

    {
        .cookie = 0x5a03,
        .memsizes = MEMSIZES(1, 0x0f, 4),
        .memranges = MEMRANGES(16, 8, 24),
        .address_pointer = 0x0000,
        .spacename = "Flash"
    },

    {
        .cookie = 0x5a04,
        .memsizes = MEMSIZES(1, 2, 2),
        .memranges = MEMRANGES(0, 0, 4),
        .address_pointer = 0x0000,
        .spacename = "Timers"
    },

    // There is no memory space 5.
    {
        .cookie = 0x0000,
        .memsizes = MEMSIZES(1, 1, 7),
        .memranges = MEMRANGES(0, 0, 0),
        .address_pointer = 0x0000,
        .spacename = "unused"
    },

    {
        .cookie = 0x5a06,
        .memsizes = MEMSIZES(1, 2, 2),
        .memranges = MEMRANGES(0, 0, 4),
        .address_pointer = 0x0000,
        .spacename = "LBP16RW"
    },

    {
        .cookie = 0x5a07,
        .memsizes = MEMSIZES(0, 2, 2),
        .memranges = MEMRANGES(0, 0, 4),
        .address_pointer = 0x0000,
        .spacename = "LBP16RO"
    },

};


// Read-only const board id.  From the 7i93 manual v1.0:
// MEMORY SPACE 7 LAYOUT:
// ADDRESS DATA
// 0000 CardNameChar-0,1
// 0002 CardNameChar-2,3
// 0004 CardNameChar-4,5
// 0006 CardNameChar-6,7
// 0008 CardNameChar-8,9
// 000A CardNameChar-10,11
// 000C CardNameChar-12.13
// 000E CardNameChar-14,15
// 0010 LBPVersion
// 0012 FirmwareVersion
// 0014 Option Jumpers
// 0016 Reserved
// 0018 RecvStartTS 1 uSec timestamps
// 001A RecvDoneTS For performance monitoring
// 001C SendStartTS Send timestamps are
// 001E SendDoneTS from previous packet
//
// Only the CardName seems to be used.  Should be all uppercase.
uint8_t const memory_space_7[32] = {
    "W5500-EVB-PICO"
};


static void set_clock_khz(void) {
    // set a system clock frequency in khz
    set_sys_clock_khz(PLL_SYS_KHZ, true);

    // configure the specified clock
    clock_configure(
        clk_peri,
        0,                                                // No glitchless mux
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, // System PLL on AUX mux
        PLL_SYS_KHZ * 1000,                               // Input frequency
        PLL_SYS_KHZ * 1000                                // Output (must be same as no divider)
    );
}


static void handle_lbp16(uint8_t const * packet, size_t size, uint8_t reply_addr[4], uint16_t reply_port) {
    for (size_t i = 0; i < size; ++i) {
        printf("0x%02x ", packet[i]);
        if (i % 8 == 7) {
            printf("\n");
        }
    }
    printf("\n");

    uint16_t raw_cmd = packet[0] | (packet[1] << 8);
    printf("decoding cmd 0x%04x\n", raw_cmd);
    packet += 2;

    lbp16_cmd_t cmd;

    lbp16_decode_cmd(raw_cmd, &cmd);

    printf("    write: %d\n", cmd.write);
    printf("    has_addr: %d\n", cmd.has_addr);
    printf("    info_area: %d\n", cmd.info_area);
    printf("    memory_space: %d\n", cmd.memory_space);
    printf("    transfer_size: %d (%d bytes, %d bits)\n", cmd.transfer_size, cmd.transfer_bytes, cmd.transfer_bits);
    printf("    addr_increment: %d\n", cmd.addr_increment);
    printf("    transfer_count: %d\n", cmd.transfer_count);

    uint16_t addr;
    if (cmd.has_addr) {
        addr = packet[0] | (packet[1] << 8);
        printf("    addr: 0x%04x\n", addr);
        packet += 2;
    } else {
        printf("no addr??\n");
        return;
    }

    if (cmd.transfer_count < 1 || cmd.transfer_count > 127) {
        printf("transfer count %d out of bounds\n", cmd.transfer_count);
        return;
    }

    if (cmd.write) {

        switch (cmd.memory_space) {

            case 0: {
                if (cmd.transfer_size != 2) {
                    printf("i only know how to write 32-bit chunks to memory area 0\n");
                    return;
                }

                for (size_t i = 0; i < cmd.transfer_count; ++i) {
                    // Lucky us, the RP2040 is little-endian just like
                    // the LBP16 network protocol.
                    memcpy(&hm2_register_file[addr], &packet[i*4], 4);
                    if (cmd.addr_increment) {
                        addr += 4;
                    }
                }

                break;
            }

            default: {
                printf("can't write to memory space %d\n", cmd.memory_space);
                break;
            }
        }

    } else {
        uint8_t reply_packet[127*4];

        switch (cmd.memory_space) {

            case 0: {
                if (cmd.transfer_size != 2) {
                    printf("i only know how to read 32-bit chunks from memory area 0\n");
                    return;
                }

                for (size_t i = 0; i < cmd.transfer_count; ++i) {
                    // Lucky us, the RP2040 is little-endian just like
                    // the LBP16 network protocol.
                    memcpy(&reply_packet[i*4], &hm2_register_file[addr], 4);
                    if (cmd.addr_increment) {
                        addr += 4;
                    }
                }
                int32_t r = sendto(0, reply_packet, cmd.transfer_count * 4, reply_addr, reply_port);
                break;
            }

            case 7: {
                if (cmd.transfer_size != 1) {
                    printf("i only know how to transfer 16-bit chunks from memory area 7\n");
                    return;
                }

                memcpy(reply_packet, memory_space_7, cmd.transfer_count * 2);
                int32_t r = sendto(0, (uint8_t *)reply_packet, cmd.transfer_count * 2, reply_addr, reply_port);
                break;
            }

            default: {
                printf("can't read from memory space %d\n", cmd.memory_space);
                break;
            }

        }
    }

}


int main() {
    stdio_init_all();

    led_blink(4, 200);

    printf("Hostmot2 firwmare starting\n");

    ioport_init();
    idrom_init();
    led_init();

    printf("Hostmot2 firmware initialized!\n");


    multicore_launch_core1(hm2_fw_run);


    set_clock_khz();

    wizchip_spi_initialize();
    wizchip_cris_initialize();

    wizchip_reset();
    wizchip_initialize();
    wizchip_check();

    network_initialize(g_net_info);

    print_network_information(g_net_info);


    int8_t sock = socket(0, Sn_MR_UDP, 27181, 0);
    printf("socket %d\n", sock);

    while (true) {
        uint8_t packet[1024];
        uint8_t addr[4];
        uint16_t port;

        int32_t r = recvfrom(0, packet, sizeof(packet), addr, &port);
        printf("recvfrom %d (addr=%u.%u.%u.%u, port=%u)\n", r, addr[0], addr[1], addr[2], addr[3], port);
        handle_lbp16(packet, r, addr, port);
    }
}
