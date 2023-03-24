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


#define PLL_SYS_KHZ (133 * 1000)


static wiz_NetInfo g_net_info = {
    .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56}, // MAC address
    .ip = {192, 168, 1, 121},                    // IP address
    .sn = {255, 255, 255, 0},                    // Subnet Mask
    .gw = {192, 168, 1, 1},                      // Gateway
    .dns = {8, 8, 8, 8},                         // DNS server
    .dhcp = NETINFO_STATIC                       // DHCP enable/disable
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
uint8_t memory_area_7[32] = {
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


static void handle_lbp16(uint8_t const * const packet, size_t size, uint8_t reply_addr[4], uint16_t reply_port) {
    for (size_t i = 0; i < size; ++i) {
        printf("0x%02x ", packet[i]);
        if (i % 8 == 7) {
            printf("\n");
        }
    }
    printf("\n");

    uint16_t cmd = packet[0] | (packet[1] << 8);
    printf("decoding cmd 0x%04x\n", cmd);

    bool cmd_write = cmd & 0x8000;
    bool cmd_has_addr = cmd & 0x4000;
    bool cmd_info_area = cmd & 0x2000;
    uint8_t cmd_memory_space = (cmd >> 10) & 0x7;
    uint8_t cmd_transfer_size = (cmd >> 8) & 0x3;
    int8_t cmd_transfer_bits;
    bool cmd_addr_increment = cmd & 0x0080;
    uint8_t cmd_transfer_count = cmd & 0x7f;

    switch (cmd_transfer_size) {
        case 0:
            cmd_transfer_bits = 8;
            break;
        case 1:
            cmd_transfer_bits = 16;
            break;
        case 2:
            cmd_transfer_bits = 32;
            break;
        case 3:
            cmd_transfer_bits = 64;
            break;
        default:
            cmd_transfer_bits = -1;
    }


    printf("    write: %d\n", cmd_write);
    printf("    has_addr: %d\n", cmd_has_addr);
    printf("    info_area: %d\n", cmd_info_area);
    printf("    memory_space: %d\n", cmd_memory_space);
    printf("    transfer_size: %d (%d bits)\n", cmd_transfer_size, cmd_transfer_bits);
    printf("    addr_increment: %d\n", cmd_addr_increment);
    printf("    transfer_count: %d\n", cmd_transfer_count);

    if (!cmd_has_addr) {
        printf("no addr??\n");
        return;
    }

    if (cmd_info_area != 0) {
        printf("what even is the info area??\n");
        return;
    }

    if (cmd_transfer_count < 1 || cmd_transfer_count > 127) {
        printf("transfer count %d out of bounds\n", cmd_transfer_count);
        return;
    }

    uint16_t addr = 0x0000;
    if (cmd_has_addr) {
        addr = packet[2] | (packet[3] << 8);
    }

    if (cmd_write) {
        /*
        for (size_t i = 0; i < cmd_transfer_count; ++i) {
            spi_read_blocking(spi_default, 0x5a, (uint8_t*)&hm2_register_file32[addr/4], 4);
            if (addr_auto_increment) {
                addr += 4;
            }
        }
        */
    } else {
        uint8_t reply_packet[127*4];

        switch (cmd_memory_space) {

            case 0: {
                if (cmd_transfer_size != 2) {
                    printf("i only know how to transfer 32-bit chunks from memory area 0\n");
                    return;
                }

                for (size_t i = 0; i < cmd_transfer_count; ++i) {
                    // Lucky us, the RP2040 is little-endian just like
                    // the LBP16 network protocol.
                    memcpy(&reply_packet[i*4], &hm2_register_file[addr], 4);
                    if (cmd_addr_increment) {
                        addr += 4;
                    }
                }
                int32_t r = sendto(0, reply_packet, cmd_transfer_count * 4, reply_addr, reply_port);
                break;
            }

            case 7: {
                if (cmd_transfer_size != 1) {
                    printf("i only know how to transfer 16-bit chunks from memory area 7\n");
                    return;
                }

                memcpy(reply_packet, memory_area_7, cmd_transfer_count * 2);
                int32_t r = sendto(0, (uint8_t *)reply_packet, cmd_transfer_count * 2, reply_addr, reply_port);
                break;
            }

            default: {
                printf("unknown memory space %d\n", cmd_memory_space);
                break;
            }

        }
    }

}


int main() {
    stdio_init_all();

    led_blink(2, 200);

    printf("Hostmot2 firwmare starting\n");

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
