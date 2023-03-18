This is an implementation of hostmot2 firmware for the RP2040
microcontroller.


# Host connection options


## Ethernet

The primary target is the Wiznet W6100, because it has Ethernet:
<https://docs.wiznet.io/Product/iEthernet/W6100/w6100-evb-pico>

$18 from Mouser: <https://www.mouser.com/ProductDetail/WIZnet/W6100-EVB-PICO?qs=amGC7iS6iy9FRNAvZsvTNg%3D%3D#>

<https://github.com/WIZnet-ArduinoEthernet/Ethernet/tree/W6100-EVB-Pico>
<https://linuxgizmos.com/wiznet-board-features-raspberry-pi-2040-and-hardwired-internet-controller-chip/>


## SPI

Possibly any RP2040 board might work, as long as the SPI is available?

On a Raspberry Pi 4 running Ubuntu 22.10 (or probably any Debian-based
RPi with working /dev/spi), install spi-tools and run something like:

`$ echo -en 'ABCD' | sudo spi-pipe --device=/dev/spidev0.0 --speed=$((10*1000*1000)) --blocksize=1 | hd`

The SPI controller in the RP2040 in slave mode tops out around 11 MHz
(RP2040 Datasheet 2023-03-02, section 4.4.3.4), which is not great.

The current RP2040 code is too slow to keep up with mesaflash at 1 MHz
SPI freq.  It's ok at 100 kHz.  SPI itself runs fine up to 10 MHz, but the
code that feeds the hm2 registers to the SPI transmit queue is too slow.
Maybe SPI DMA would help?  Certainly going from SPI to Ethernet will
fix this particular timing problem.

Sniff & decode SPI with pulseview/sigrok, though my ancient Saleae Logic
tops out around 8 MHz, so not useful for the 20 MHz that mesaflash uses,
or the 30+ MHz that hostmot2 likes to use.


## USB3

The RP2040 only has USB 1.1, no possibly low-latency USB 3.  :-(


### Infineon EZ-USB FX3

This stand-alone USB3-to-GPIF chip (whatever GPIF is) costs $40:
<https://www.infineon.com/cms/en/product/universal-serial-bus/usb-peripheral-controllers-for-superspeed/ez-usb-fx3-usb-5gbps-peripheral-controller/>


### Infineon CYUSB3065-BZXC

Infineon/Cypress chip with Arm9 (200
MHz), USB3 peripheral, and 12 gpios, $30:
<https://www.mouser.com/ProductDetail/Infineon-Technologies/CYUSB3065-BZXC?qs=bWsWnctWn1zH454yc%252Bd8Ww%3D%3D>


### FTDI FT60xQ-B

FTDI FT601 costs $10: <https://ftdichip.com/products/ft601q-b/>


### TI TUSB921

TI TUSB9261, Arm Cortex M3, USB3 peripherial, "up to 12
gpios", $8, designed for storage (SATA) applications:
<https://www.mouser.com/ProductDetail/Texas-Instruments/TUSB9261IPVP?qs=FBI%252BX3tnPf1SUg0QkfZf9Q%3D%3D>

There's a TUSB921 "demo board" available for $65.


### WCH CH569

Vapor?

WCH CH569:

    <http://www.wch-ic.com/products/CH569.html>

    <https://www.cnx-software.com/2020/07/21/wch-ch569-risc-v-soc-offers-usb-3-0-gigabit-ethernet-high-speed-serdes-hspi-interfaces/>


### USB3 stack for STM32

<https://github.com/xtoolbox/TeenyUSB>




# RP2040

<https://www.raspberrypi.com/documentation/microcontrollers/rp2040.html>


## C SDK

<https://www.raspberrypi.com/documentation/microcontrollers/c_sdk.html>

Examples here: <https://github.com/raspberrypi/pico-examples/>


### C SDK builder docker image

In the hm2-rp2040 docker container (with this repo mounted in it):

```
$ mkdir build
$ cd build
$ cmake -DPICO_BOARD=adafruit_feather_rp2040 ..
$ make -C firmware -j$(nproc --all)
```

Switch the RP2040 to bootloader mode.  For example, on the Adafruit
Feather RP2040: hold Boot button, click Reset button, release Boot button.

On the host (assuming the RP2040 boot partition shows up as `/dev/sda`):

```
$ sudo mount /dev/sda1 /mnt
$ sudo cp firmware/*.uf2 /mnt
$ sudo umount /mnt
```


## PIO

<https://learn.adafruit.com/intro-to-rp2040-pio-with-circuitpython>




# Adafruit RP2040 Scorpio

I'm prototyping on the Adafruit RP2040 Scorpio (because that's what I
have on hand).

<https://learn.adafruit.com/introducing-feather-rp2040-scorpio>

Uses PIO to control WS2812 neopixels.
