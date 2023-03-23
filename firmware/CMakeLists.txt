add_library(
    hostmot2_firmware
    hm2-fw.c
    idrom.c
    led.c
)

target_link_libraries(
    hostmot2_firmware
    PRIVATE
    pico_stdlib
    hardware_dma
)


#
# SPI on Adafruit Feather RP2040 Scorpio
# PICO_BOARD="adafruit_feather_rp2040"
#

add_executable(
    hm2_fw_spi
    hm2_fw_spi.c
)

add_compile_definitions(
    hm2_fw_spi
    PRIVATE
    PICO_DEFAULT_SPI=1
    PICO_DEFAULT_SPI_SCK_PIN=14 # labeled SCK (green)
    PICO_DEFAULT_SPI_TX_PIN=8   # labeled MI (blue)
    PICO_DEFAULT_SPI_RX_PIN=15  # labeled MO (yellow)
    PICO_DEFAULT_SPI_CSN_PIN=9  # labeled 9 (white)
)

# Pull in basic dependencies
target_link_libraries(
    hm2_fw_spi
    pico_stdlib
    pico_multicore
    hardware_spi
    hostmot2_firmware
)

# enable usb output, disable uart output
pico_enable_stdio_usb(hm2_fw_spi 1)
pico_enable_stdio_uart(hm2_fw_spi 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(hm2_fw_spi)


#
# Wiznet W5500-EVB-Pico
# PICO_BOARD="wiznet_w5100s_evb_pico"
#

add_executable(
    hm2_fw_eth_w5500
    hm2_fw_eth_w5500.c
)

target_link_libraries(
    hm2_fw_eth_w5500
    PRIVATE
    pico_stdlib
    pico_multicore
    hardware_spi
    hardware_dma
    ETHERNET_FILES
    IOLIBRARY_FILES
    hostmot2_firmware
)

pico_enable_stdio_usb(hm2_fw_eth_w5500 1)
pico_enable_stdio_uart(hm2_fw_eth_w5500 0)

pico_add_extra_outputs(hm2_fw_eth_w5500)
