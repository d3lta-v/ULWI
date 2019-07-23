#!/bin/bash

# This script automates the flashing of an ESP8266 chip

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
FIRMWARE="release/fw.zip"
PORT="/dev/cu.SLAB_USBtoUART"

mos flash --firmware "$DIR/$FIRMWARE" --port "$PORT"
