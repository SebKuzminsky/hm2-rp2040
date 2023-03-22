// Copyright (c) 2021 Michael Stoops. All rights reserved.
// Portions copyright (c) 2021 Raspberry Pi (Trading) Ltd.
// 
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the 
// following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following
//    disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
//    following disclaimer in the documentation and/or other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote
//    products derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// SPDX-License-Identifier: BSD-3-Clause
//
// Example of an SPI bus slave using the PL022 SPI interface

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/multicore.h"
#include "hardware/spi.h"

#include "hm2-fw.h"


#if !defined(spi_default) || !defined(PICO_DEFAULT_SPI_SCK_PIN) || !defined(PICO_DEFAULT_SPI_TX_PIN) || !defined(PICO_DEFAULT_SPI_RX_PIN) || !defined(PICO_DEFAULT_SPI_CSN_PIN)
#error hm2-fw requires a board with SPI pins
#endif

#if !defined(PICO_DEFAULT_LED_PIN)
#error hm2-fw requires a board with an LED pin
#endif


#define HM2_SPI_CMD_READ  (0xa)
#define HM2_SPI_CMD_WRITE (0xb)


void printbuf(uint8_t buf[], size_t len) {
    int i;
    for (i = 0; i < len; ++i) {
        if (i % 16 == 15)
            printf("%02x\n", buf[i]);
        else
            printf("%02x ", buf[i]);
    }

    // append trailing newline if there isn't one
    if (i % 16) {
        putchar('\n');
    }
}


int main() {
    // Enable stdio so we can print log/debug messages.
    stdio_init_all();


    // Blink the LED for a while, so the host has time to notice the USB
    // serial port and connect to it, so it can see the initialization
    // log messages...
    uint const LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_put(LED_PIN, 1);
    sleep_ms(200);
    gpio_put(LED_PIN, 0);
    sleep_ms(200);
    gpio_put(LED_PIN, 1);
    sleep_ms(200);
    gpio_put(LED_PIN, 0);
    sleep_ms(200);

    gpio_put(LED_PIN, 1);
    sleep_ms(200);
    gpio_put(LED_PIN, 0);
    sleep_ms(200);
    gpio_put(LED_PIN, 1);
    sleep_ms(200);
    gpio_put(LED_PIN, 0);
    sleep_ms(200);


    printf("Hostmot2 firwmare starting\n");

    idrom_init();
    led_init();

    printf("Hostmot2 firmware initialized!\n");


    multicore_launch_core1(hm2_fw_run);


    // Enable SPI at 10 MHz and connect to GPIOs
    spi_init(spi_default, 10 * 1000 * 1000);
    spi_set_slave(spi_default, true);
    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI);

    // THIS LINE IS ABSOLUTELY KEY. Enables multi-byte transfers with
    // one CS assert.  See section 4.4.3 of the RP2040 Datasheet.
    spi_set_format(spi_default, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);

    // Make the SPI pins available to picotool
    bi_decl(bi_4pins_with_func(PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI));

    if (spi_is_readable(spi_default)) {
        printf("draining SPI read queue\n");
        while (spi_is_readable(spi_default)) {
            uint8_t garbage;
            spi_read_blocking(spi_default, 0xAA, &garbage, 1);
            printbuf(&garbage, 1);
        }
    }

    // Main loop
    while (true) {
        uint8_t cmd_frame[4];

        // Read a command frame from the control computer (while writing
        // some garbage that will be ignored).
        spi_read_blocking(spi_default, 0x5A, cmd_frame, 4);
        // printbuf(cmd_frame, 4);

        uint16_t addr = ((uint16_t)cmd_frame[0] << 8) | (cmd_frame[1]);
        int cmd = 0x0f & (cmd_frame[2] >> 4);
        bool addr_auto_increment = 0x1 & (cmd_frame[2] >> 3);
        size_t size = 0x7f & ((((uint16_t)cmd_frame[2] << 8) | (cmd_frame[3])) >> 4);  // `size` is the number of 32-bit words to read or write
        // printf("cmd frame:\n    addr=0x%04x\n    cmd=0x%1x\n    addr_auto_increment=%d\n    size=%d\n", addr, cmd, addr_auto_increment, size);

        if (cmd == HM2_SPI_CMD_READ) {
            for (size_t i = 0; i < size; ++i) {
                // printf("read 4 bytes from 0x%04x\n", addr);
                uint8_t garbage[4];
                spi_write_read_blocking(spi_default, &hm2_register_file[addr], (uint8_t*)&garbage, 4);
                if (addr_auto_increment) {
                    addr += 4;
                }
            }
        }

        if (cmd == HM2_SPI_CMD_WRITE) {
            for (size_t i = 0; i < size; ++i) {
                spi_read_blocking(spi_default, 0x5a, (uint8_t*)&hm2_register_file32[addr/4], 4);
                if (addr_auto_increment) {
                    addr += 4;
                }
            }
        }
    }
}
