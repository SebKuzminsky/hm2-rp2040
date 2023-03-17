#ifndef HM2_FW_H
#define HM2_FW_H


#define HM2_MAX_REGIONS 8


#define htonl(x) (((x & 0xff000000) >> 24) | ((x & 0x00ff0000) >> 8) | ((x & 0x0000ff00) << 8) | ((x & 0x000000ff) << 24))


typedef struct {
    char const * name;
    uint16_t addr;
    size_t size;
    void (*update)(void);
} hm2_region_t;

extern hm2_region_t hm2_region[HM2_MAX_REGIONS];
extern size_t hm2_num_regions;

extern uint8_t hm2_register_file[1<<16];


uint8_t * hm2_fw_register(
    char const * const name,
    uint16_t const addr,
    size_t const size,
    void (*update)(void)
);


int idrom_init(void);


#endif // HM2_FW_H
