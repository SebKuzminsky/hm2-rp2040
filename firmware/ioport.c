#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "hm2-fw.h"


// 0x1000  I/O port  0..23
// 0x1004  I/O port 24..47
// 0x1008  I/O port 48..71
// 0x100C  I/O port 72..95
// 0x1010  I/O port 96..127
// 0x1014  I/O port 128..143
//
//     Writes write to output register, reads read pin status
//
// 0x1100  DDR for I/O port  0..23
// 0x1104  DDR for I/O port  24..47
// 0x1108  DDR for I/O port  48..71
// 0x110C  DDR for I/O port  72..95
// 0x1110  DDR for I/O port  96..127
// 0x1114  DDR for I/O port  128..144
//
//     '1' bit in DDR register makes corresponding GPIO bit an output
//
// 0x1200  AltSourceReg for I/O port  0..23
// 0x1204  AltSourceReg for I/O port  24..47
// 0x1208  AltSourceReg for I/O port  48..71
// 0x120C  AltSourceReg for I/O port  72..95
// 0x1210  AltSourceReg for I/O port  96..127
// 0x1214  AltSourceReg for I/O port  128..143
//
//     '1' bit in AltSource register makes corresponding GPIO bit data source
//     come from Alternate source for that bit instead of GPIO output register.
//
// 0x1300  OpenDrainSelect for I/O port  0..23
// 0x1304  OpenDrainSelect for I/O port  24..47
// 0x1308  OpenDrainSelect for I/O port  48..71
// 0x130C  OpenDrainSelect for I/O port  72..95
// 0x1310  OpenDrainSelect for I/O port  96..127
// 0x1314  OpenDrainSelect for I/O port  128..143
//
//     '1' bit in OpenDrainSelect register makes corresponding GPIO an
//     open drain output.
//     If OpenDrain is selected for an I/O bit , the DDR register is ignored.
//
// 0x1400  OutputInvert for I/O port  0..23
// 0x1404  OutputInvert for I/O port  24..47
// 0x1408  OutputInvert for I/O port  48..71
// 0x140C  OutputInvert for I/O port  72..95
// 0x1410  OutputInvert for I/O port  96..127
// 0x1414  OutputInvert for I/O port  128..143
//
//     A '1' bit in the OutputInv register inverts the cooresponding output bit.
//     This may be the GPIO output register bit or alternate source. The input is
//     not inverted.


static uint32_t * reg;


//
// A "1" bit indicates the corresponding GPIO line is available, "0"
// indicates it's not.
//
// The RP2040 has 29 GPIO lines.  On the W5500-EVB-Pico, GPIOs 0-15,
// 22, and 26-28 are available for GPIO.
//
// The bitmaps are:
// GPIO 00-23: 0000 0000  0100 0000  1111 1111  1111 1111
// GPIO 24-29: 0000 0000  0000 0000  0000 0000  0001 1100
//

static uint32_t lines_available[2] = { 0x0040ffff, 0x0000001c };


//
// Data direction register.
//
// A "1" bit indicates the corresponding GPIO line is an output, "0"
// indicates it's an input.
//

static uint32_t ddr[2] = { 0, 0 };

// Output value register.
static uint32_t output_val[2] = { 0, 0 };


// Set GPIO directions based on ddr.
static void update_ddr(void) {
    for (size_t i = 0; i < 29; ++i) {
        int instance = i / 24;
        int num_in_instance = i % 24;

        if (lines_available[instance] & (1 << num_in_instance)) {
            if (ddr[instance] & (1 << num_in_instance)) {
                gpio_set_dir(i, GPIO_OUT);
            } else {
                gpio_set_dir(i, GPIO_IN);
            }
        }
    }
}


static void update_outputs(void) {
    uint32_t mask;
    uint32_t val;

    mask = lines_available[0] | (lines_available[1] << 24);
    mask &= ddr[0] | (ddr[1] << 24);

    val = output_val[0] | (output_val[1] << 24);

    gpio_put_masked(mask, val);
}


static int ioport_write(uint16_t addr, uint32_t const * buf, size_t num_uint32) {
    // printf("%s: addr=0x%04x, num_uint32=%u\n", __FUNCTION__, addr, num_uint32);
    // log_uint32(buf, num_uint32);

    if (addr < 0x0100) {
        // Write the GPIO outputs.
        if (addr >= 2) {
            return -1;
        }
        for (size_t i = 0; i < num_uint32; ++i) {
            output_val[addr + i] = buf[i];
        }
        update_outputs();

    } else if (addr < 0x0200) {
        // Write the DDR (data direction) register.
        uint16_t ddr_addr = addr - 0x0100;
        if (ddr_addr >= 2) {
            return -1;
        }
        for (size_t i = 0; i < num_uint32; ++i) {
            ddr[ddr_addr + i] = buf[i];
        }
        update_ddr();
        return 0;
    }

    return -1;
}


static int ioport_read(uint16_t addr, uint32_t * buf, size_t num_uint32) {
    if (addr < 0x0100) {
        // Read GPIO inputs.
        uint32_t in_values = gpio_get_all();
        uint32_t p[2];
        p[0] = in_values & lines_available[0];
        p[1] = (in_values >> 24) & lines_available[1];
        for (size_t i = 0; i < num_uint32; ++i) {
            buf[i] = p[i];
        }
        return 0;

    } else if (addr < 0x0200) {
        // Read the DDR (data direction) register.
        for (size_t i = 0; i < num_uint32; ++i) {
            buf[i] = ddr[i];
        }
        return 0;
    }

    return -1;
}


int ioport_init(void) {
    for (size_t i = 0; i < 29; ++i) {
        int instance = i / 24;
        int num_in_instance = i % 24;

        if (lines_available[instance] & (1 << num_in_instance)) {
            printf("initializing GPIO%d for input, pulled down\n", i);
            gpio_init(i);
            gpio_set_function(i, GPIO_FUNC_SIO);
            gpio_set_dir(i, GPIO_IN);
            gpio_pull_down(i);
        }
    }

    reg = (uint32_t *)hm2_fw_register("ioport", 0x1000, 0x500, NULL, ioport_write, ioport_read);
    if (reg == NULL) {
        return -1;
    }
    return 0;
}
