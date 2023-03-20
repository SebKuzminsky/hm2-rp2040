/**
 * Copyright (c) 2021 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>

#include "port_common.h"

#include "wizchip_conf.h"
#include "w5x00_spi.h"

#include "socket.h"

// #include "httpServer.h"
// #include "web_page.h"


/* Clock */
#define PLL_SYS_KHZ (133 * 1000)

/* Buffer */
// #define ETHERNET_BUF_MAX_SIZE (1024 * 2)

/* Socket */
// #define HTTP_SOCKET_MAX_NUM 4

/* Network */
static wiz_NetInfo g_net_info = {
    .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56}, // MAC address
    .ip = {192, 168, 1, 121},                    // IP address
    .sn = {255, 255, 255, 0},                    // Subnet Mask
    .gw = {192, 168, 1, 1},                      // Gateway
    .dns = {8, 8, 8, 8},                         // DNS server
    .dhcp = NETINFO_STATIC                       // DHCP enable/disable
};

/* HTTP */
// static uint8_t g_http_send_buf[ETHERNET_BUF_MAX_SIZE] = {
    // 0,
// };
// static uint8_t g_http_recv_buf[ETHERNET_BUF_MAX_SIZE] = {
    // 0,
// };
// static uint8_t g_http_socket_num_list[HTTP_SOCKET_MAX_NUM] = {0, 1, 2, 3};


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


int main() {
    stdio_init_all();

    uint const led_pin = 25;
    gpio_init(led_pin);
    gpio_set_dir(led_pin, GPIO_OUT);

    gpio_put(led_pin, 1);
    sleep_ms(200);
    gpio_put(led_pin, 0);
    sleep_ms(200);
    gpio_put(led_pin, 1);
    sleep_ms(200);
    gpio_put(led_pin, 0);
    sleep_ms(200);

    gpio_put(led_pin, 1);
    sleep_ms(200);
    gpio_put(led_pin, 0);
    sleep_ms(200);
    gpio_put(led_pin, 1);
    sleep_ms(200);
    gpio_put(led_pin, 0);
    sleep_ms(200);

    printf("hello world\n");


    set_clock_khz();

    wizchip_spi_initialize();
    wizchip_cris_initialize();

    wizchip_reset();
    wizchip_initialize();
    wizchip_check();

    network_initialize(g_net_info);

    print_network_information(g_net_info);


    // httpServer_init(g_http_send_buf, g_http_recv_buf, HTTP_SOCKET_MAX_NUM, g_http_socket_num_list);
    // reg_httpServer_webContent("index.html", index_page);
    // while (1) {
    //     for (uint8_t i = 0; i < HTTP_SOCKET_MAX_NUM; i++) {
    //         httpServer_run(i);
    //     }
    // }

    int8_t sock = socket(0, Sn_MR_UDP, 27181, 0);
    printf("socket %d\n", sock);

    while (true) {
        uint8_t packet[1024];
        uint8_t addr[4];
        uint16_t port;

        int32_t r = recvfrom(0, packet, sizeof(packet), addr, &port);
        printf("recvfrom %d (addr=%u.%u.%u.%u, port=%u)\n", r, addr[0], addr[1], addr[2], addr[3], port);

        for (int i = 0; i < r; ++i) {
            printf("0x%02x (%c) ", packet[i], packet[i]);
            if (i % 8 == 7) {
                printf("\n");
            }
        }
        printf("\n");
    }
}
