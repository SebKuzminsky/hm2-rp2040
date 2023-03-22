add_library(
    hostmot2-firmware
    hm2-fw.c
    idrom.c
    led.c
)

target_link_libraries(
    hostmot2-firmware
    PRIVATE
    pico_stdlib
    hardware_dma
)


#
# SPI
#

add_executable(
    hm2_fw_spi
    spi_slave.c
)

# Pull in basic dependencies
target_link_libraries(
    hm2_fw_spi
    pico_stdlib
    pico_multicore
    hardware_spi
    hostmot2-firmware
)

# enable usb output, disable uart output
pico_enable_stdio_usb(hm2_fw_spi 1)
pico_enable_stdio_uart(hm2_fw_spi 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(hm2_fw_spi)


#
# Wiznet W5500
#

set(TARGET_NAME hm2_fw_eth_w5500)

add_executable(
    ${TARGET_NAME}
    ${TARGET_NAME}.c
)

target_link_libraries(
    ${TARGET_NAME} PRIVATE
    pico_stdlib
    hardware_spi
    hardware_dma
    ETHERNET_FILES
    IOLIBRARY_FILES
    HTTPSERVER_FILES
    hostmot2-firmware
)

pico_enable_stdio_usb(${TARGET_NAME} 1)
pico_enable_stdio_uart(${TARGET_NAME} 0)

pico_add_extra_outputs(${TARGET_NAME})
