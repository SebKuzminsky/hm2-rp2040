#include <stdio.h>
#include "pico/stdlib.h"

#include "hm2-fw.h"


hm2_region_t hm2_region[HM2_MAX_REGIONS];
size_t hm2_num_regions;

uint8_t hm2_register_file[1<<16];
uint32_t * hm2_register_file32 = (uint32_t *)hm2_register_file;


uint8_t * hm2_fw_register(
    char const * name,
    uint16_t addr,
    size_t size,
    void (*module_update)(void),
    void (*module_write)(uint16_t addr, uint32_t val),
    uint32_t (*module_read)(uint16_t addr)
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
