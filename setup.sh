#!/bin/sh -e
# turn off LEDs 1 - 3, only 0 will be on
echo 0 > /sys/class/leds/beaglebone:green:usr1/brightness
echo 0 > /sys/class/leds/beaglebone:green:usr2/brightness
echo 0 > /sys/class/leds/beaglebone:green:usr3/brightness
 
 
# configure P9 GPIO ports
config-pin p9.12 gpio
config-pin p9.15 gpio

# configure I2C (i2c2) ports
config-pin p9.19 i2c	# scl
config-pin p9.20 i2c	# sda

# configure CAN ports
# P9_19 -> dCAN0 Rx
# P9_20 -> dCAN0 Tx
# P9_24 -> dCAN1 Rx
# P9_26 -> dCAN1 Tx
# config-pin p9.19 can
# config-pin p9.20 can
# config-pin p9.24 can
# config-pin p9.26 can

#SPI setup
config-pin p9.28 spi_cs
config-pin p9.29 spi
config-pin p9.30 spi
config-pin p9.31 spi_sclk
 
exit 0
