add_executable(
    hm2_fw_spi
    spi_slave.c
    hm2-fw.c
    idrom.c
    led.c
)

# Pull in basic dependencies
target_link_libraries(hm2_fw_spi pico_stdlib pico_multicore hardware_spi)

# enable usb output, disable uart output
pico_enable_stdio_usb(hm2_fw_spi 1)
pico_enable_stdio_uart(hm2_fw_spi 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(hm2_fw_spi)
