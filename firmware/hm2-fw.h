#ifndef HM2_FW_H
#define HM2_FW_H


#define HM2_GTAG_IOPORT  3
#define HM2_GTAG_STEPGEN 5

#define HM2_GTAG_END     0


#define HM2_MAX_REGIONS 8


typedef struct {
    char const * name;
    uint16_t addr;
    size_t size;
    void (*update)(void);
} hm2_region_t;

extern hm2_region_t hm2_region[HM2_MAX_REGIONS];
extern size_t hm2_num_regions;

extern uint8_t hm2_register_file[1<<16];
extern uint32_t * hm2_register_file32;


uint8_t * hm2_fw_register(
    char const * const name,
    uint16_t const addr,
    size_t const size,
    void (*update)(void)
);


void hm2_fw_run(void);


int idrom_init(void);
int ioport_init(void);
int led_init(void);


// This is a helper function to blink the LED, to show that the hostmot2
// firmware is booting.
void led_blink(uint8_t const num_blinks, uint16_t const ms_delay);


#endif // HM2_FW_H
