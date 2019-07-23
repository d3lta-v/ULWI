#!/bin/bash

# This script automates the flashing of an ESP8266 chip

FIRMWARE="release/fw.zip"
PORT="/dev/cu.SLAB_USBtoUART"

mos flash --firmware "$FIRMWARE" --port "$PORT"
