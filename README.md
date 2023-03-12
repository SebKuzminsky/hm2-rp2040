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


## USB3

The RP2040 only has USB 1.1, no possibly low-latency USB 3.  :-(

This stand-alone USB3-to-GPIF chip (whatever GPIF is) costs $40:
<https://www.infineon.com/cms/en/product/universal-serial-bus/usb-peripheral-controllers-for-superspeed/ez-usb-fx3-usb-5gbps-peripheral-controller/>

<https://github.com/xtoolbox/TeenyUSB>




# RP2040

<https://www.raspberrypi.com/documentation/microcontrollers/rp2040.html>


## C SDK

<https://www.raspberrypi.com/documentation/microcontrollers/c_sdk.html>

Examples here: <https://github.com/raspberrypi/pico-examples/>


### C SDK builder docker image

In the pico-rp2040-build docker container:

```
$ cd pico-example
$ mkdir build
$ cd build
$ cmake -DPICO_BOARD=adafruit_feather_rp2040 ..
$ make -C blink -j7
```

On the Adafruit Feather RP2040: hold Boot button, click Reset button,
release Boot button.

On the host:
```
$ sudo mount /dev/sda1 /mnt
$ sudo cp blink/blink.uf2 /mnt
$ sudo umount /mnt
```


## PIO

<https://learn.adafruit.com/intro-to-rp2040-pio-with-circuitpython>




# Adafruit RP2040 Scorpio

I'm prototyping on the Adafruit RP2040 Scorpio (because that's what I
have on hand).

<https://learn.adafruit.com/introducing-feather-rp2040-scorpio>

Uses PIO to control WS2812 neopixels.
