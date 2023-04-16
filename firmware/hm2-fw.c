#include <stdio.h>
#include "pico/stdlib.h"

#include "hm2-fw.h"


hm2_region_t hm2_region[HM2_MAX_REGIONS];
size_t hm2_num_regions;

uint8_t hm2_register_file[1<<16];
uint32_t * hm2_register_file32 = (uint32_t *)hm2_register_file;


void hm2_fw_log_uint8(uint8_t const * const data, size_t num_uint8) {
    size_t i;
    for (i = 0; i < num_uint8; ++i) {
        printf("0x%02x ", data[i]);
        if (i % 8 == 7) {
            printf("\n");
        }
    }
    if (i % 8 != 7) {
        printf("\n");
    }
}


void hm2_fw_log_uint32(uint32_t const * const data, size_t num_uint32) {
    size_t i;
    for (i = 0; i < num_uint32; ++i) {
        printf("0x%08x ", data[i]);
        if (i % 8 == 7) {
            printf("\n");
        }
    }
    if (i % 8 != 7) {
        printf("\n");
    }
}


uint8_t * hm2_fw_register(
    char const * name,
    uint16_t addr,
    size_t size,
    void (*module_update)(void),
    int (*module_write)(uint16_t addr, uint32_t const * buf, size_t num_uint32),
    int (*module_read)(uint16_t addr, uint32_t * buf, size_t num_uint32)
) {
    // Register a handler for a region of the address space.
    if (hm2_num_regions >= HM2_MAX_REGIONS) {
        printf("Failed to register hm2 region handler, array is full.");
        return NULL;
    }

    printf("registering region %d (%s): addr=0x%04x, size=%u\n", hm2_num_regions, name, addr, size);

    hm2_region[hm2_num_regions].name = name;
    hm2_region[hm2_num_regions].addr = addr;
    hm2_region[hm2_num_regions].size = size;
    hm2_region[hm2_num_regions].update = module_update;
    hm2_region[hm2_num_regions].write = module_write;
    hm2_region[hm2_num_regions].read = module_read;

    ++hm2_num_regions;

    return &hm2_register_file[addr];
}


// Read `num_uint32` uint32_t values from `addr` into `buf`.
//
// Returns -1 on failure (caller should read directly from the hm2
// register file memory).
//
// Returns 0 on success.

int hm2_fw_read(uint16_t addr, uint32_t * buf, size_t num_uint32) {
    return -1;
}


// Write `num_uint32` uint32_t values from `buf` into `addr`.
//
// Returns -1 on failure (caller should write directly to the hm2 register
// file memory).
//
// Returns 0 on success.

int hm2_fw_write(uint16_t addr, uint32_t const * buf, size_t num_uint32) {
    return -1;
}


void hm2_fw_run(void) {
    while (true) {
        for (size_t i = 0; i < hm2_num_regions; ++i) {
            if (hm2_region[i].update != NULL) {
                hm2_region[i].update();
            }
        }
        // sleep_ms(1);
    }
}
