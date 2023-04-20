This is an implementation of hostmot2 firmware for the RP2040
microcontroller.




# Current status

Connects to a slightly modified mesaflash.

Connects to a slightly modified hostmot2 driver in LinuxCNC.

GPIO inputs and outputs work.

Nothing else is implemented yet.




# Implementation details

The RP2040 has two cores.  This firmware uses one core for host
communication (Ethernet or SPI) and the other core for running the
hostmot2 functionality.

The two cores communicate via shared memory: the 64 kB hostmot2 register
file.

At startup, boot core initializes the register file with the IDROM,
Module Descriptors, Pin Descriptors, and all the per-Module areas.
A handler is registered for each selected (compiled-in) Module, to
perform that Module's computation and I/O.  The second core is started,
running all the Module handlers in a loop.  Finally the boot core starts
communications with the host.


## Endian-ness

Most hm2 registers are 32 bits wide.  The registers in the register file
are stored in native (host) byte order of the RP2040's ARM Cortex-M0+
processors.  The contents of multi-byte registers are converted to the
byte order specified by the host communication interface (if needed)
when data is read from and written to the host.


## Limitations

This architecture (Eth/SPI host communications on one core, hm2 firmware
on the other core) can't handle FIFO registers.  It would need special
handling for FIFOs, which is currently not implemented.  This does not
affect any of the common simple Modules, such as ioport (gpio), stepgen,
pwmgen, or encoder.


## GPIO aka I/O Port

The RP2040 has 29 GPIO lines.  Hostmot2 supports up to 24 GPIO lines
per I/O Port instance, so we split the 29 GPIO lines into two I/O Port
instances.  The first instance handles GPIOs 0-23, the second instance
handles 24-29.

The W5500-EVB-Pico board uses GPIOs 16-21 to connect to the Ethernet
chip; GPIO 23 is not connected, and GPIO29 is connected to the +3.3V
supply rail.  So on this board we have GPIOs 0-15 and 22 on I/O Port
instance 0, and GPIOs 24-28 on instance 1.




# Host connection options


## Ethernet

The primary target is the Wiznet W5500-EVB-Pico, an
inexpensive RP2040 Pico-like boards with Ethernet:
<https://docs.wiznet.io/Product/iEthernet/W5500/w5500-evb-pico>

$10 from Digikey:
<https://www.digikey.com/en/products/detail/wiznet/W5500-EVB-PICO/16515824>

Older Wiznet boards and the upcoming W6100-EVB-Pico may also be an option:
<https://docs.wiznet.io/Product/iEthernet/W6100/w6100-evb-pico>

$18 from Mouser: <https://www.mouser.com/ProductDetail/WIZnet/W6100-EVB-PICO?qs=amGC7iS6iy9FRNAvZsvTNg%3D%3D#>

<https://github.com/WIZnet-ArduinoEthernet/Ethernet/tree/W6100-EVB-Pico>
<https://linuxgizmos.com/wiznet-board-features-raspberry-pi-2040-and-hardwired-internet-controller-chip/>

Code here: <https://github.com/Wiznet/RP2040-HAT-C>

Test ethernet performance (assuming CPU 3 is your isolcpu one):

`$ sudo taskset -c 3 chrt 99 ping -i .001 -q 192.168.1.121`

Send a UDP message to a HM2 board:

`$ printf 'hey' | nc -u 192.168.1.121 27181`

Sniff packets to see what's being sent (remember LBP16 commands are 16
bits long and sent with little-endian byte order):

`$ sudo tshark -i enx0023575c0964 -l -n -e ip.src -e udp.srcport -e ip.dst -e udp.dstport -e data -Tfield udp port 27181`

The LinuxCNC utility program `elbpcom` is also quite useful:

`$ elbpcom --address=0x104 --read=8`

`$ elbpcom --address=0x200 --write 00000080`


## SPI

Possibly any RP2040 board might work, as long as the SPI is available?

On a Raspberry Pi 4 running Ubuntu 22.10 (or probably any Debian-based
RPi with working /dev/spi), install spi-tools and run something like:

`$ echo -en 'ABCD' | sudo spi-pipe --device=/dev/spidev0.0 --speed=$((10*1000*1000)) --blocksize=1 | hd`

Write a bit to hm2 addr 0x0200, to turn the LED on:

`$ printf '\x02\x00\xb8\x10\x00\x00\x00\x80' | sudo spi-pipe --device=/dev/spidev0.0 --speed=$((100 * 1000)) --blocksize=8  | hd`

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

Dev board for $80: <https://ftdichip.com/products/umft601x-b/>


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




# Target boards


## Adafruit RP2040 Scorpio

I'm prototyping on the Adafruit RP2040 Scorpio (because that's what I
have on hand).

<https://learn.adafruit.com/introducing-feather-rp2040-scorpio>

Uses PIO to control WS2812 neopixels.


## Wiznet W5500 EVB-Pico

<https://www.wiznet.io/product-item/w5500-evb-pico/>

<https://forum.wiznet.io/>

<https://github.com/Wiznet/RP2040-HAT-C/blob/main/getting_started.md>

* clone <https://github.com/Wiznet/RP2040-HAT-C.git> including submodules

    * seems like this clones raspberrypi pico-sdk and pico-extras instead of the systemwide one, bad manners

    * applies a patch to one of their own files??

* edit CMakeLists.txt to set WIZNET_CHIP to W5500

* start with examples/http/server

* (not needed for W5500-EVB-Pico) setup SPI port and pin in 'w5x00_spi.h' in 'RP2040-HAT-C/port/ioLibrary_Driver/' directory.

* Setup network configuration in 'RP2040-HAT-C/examples/http/server//w5x00_http_server.c'

* `mkdir build && cd build`

* `cmake -DPICO_BOARD=wiznet_w5100s_evb_pico ..`

To flash: remove power, push & hold BOOTSEL button, reapply power, release BOOTSEL

This is integrated with the docker build env now.

Good ping times on Buster, terrible on Bookworm:

    --- 192.168.1.121 ping statistics ---
    1000000 packets transmitted, 1000000 received, 0% packet loss, time 1998ms
    rtt min/avg/max/mdev = 0.041/0.043/0.088/0.003 ms

