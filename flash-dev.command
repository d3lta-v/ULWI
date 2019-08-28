#!/bin/bash

# This script automates the flashing of an ESP8266 chip
# This script flashes the DEVELOPMENT build of ULWI and should not be used in production!

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
FIRMWARE="release/fw-dev.zip"
PORT="/dev/cu.SLAB_USBtoUART"

mos flash --firmware "$DIR/$FIRMWARE" --port "$PORT"
