# Ultra Light Wi-Fi Interface Firmware - ULWI

**This firmware is in BETA and is incomplete**  
The ULWI firmware aims to convert the original AT command line interface of the ESP8266 into an extremely lightweight and robust command-reply style interface, with a command set inspired by assembly mnemonics.

This firmware is developed for devices that intend to interface to the ESP8266 in a master-slave configuration, whereas the device in question acts as the master while the ESP8266 acts as the slave.

This firmware is designed to be optimised for the SPEEEduino and SSTuino family of Arduino-ESP8266 hybrid boards, with a 9600/8-N-1 software serial link connecting the ATmega328 with the ESP8266, with an ATmega328P acting as the master to the ESP8266.

## Deficiencies of the AT command interface and how to fix them

The original AT command interface or firmware used by many ESP8266 modules has many issues and we aim to mitigate them as much as possible in this firmware:

### Overly long commands

**Problem**: Commands in the original AT command interface are too long, requiring many bytes to store, load and manipulate them in a limited microcontroller like the ATmega328P. This is completely unnecessary and can be greatly shortened.

**Solution**: Use very short 3 to 5 character assembly mnemonic style commands, such as changing `AT+CWJAP_DEF` to something like `CAP` (**C**onnect to **A**ccess **P**oint).

### Eliminating synchronous/blocking commands

**Problem**: Many commands in the AT command interface are "blocking" or synchronous, meaning that they require the master to stop and wait for a reply while the ESP8266 performs a long-running task that is asynchronous in nature, such as connecting to an access point or initiating a TCP request.

**Solution**: Shorten the execution time of all commands by immediately replying the master and restructure the master’s code to be a polling architecture. For example, instead of waiting for a TCP request to finish, we can initiate the TCP request first, do other things, and issue a periodic command to the ESP8266 to check if the TCP command has finished.

## Proposed enhancements to the AT command interface firmware

This firmware is also intended not just to fix the deficiencies of the old one, but also to propose new possibilities that improves on the original firmware. Goals marked as (Extended) are goals that may not be implemented in the first major release version due to its complexity.

### Easy to use HTTP/REST request and response interface

Develop an easy to use interface for initiating HTTP/REST based requests, which is very common in IoT applications.

### TLSv1.2 support

Add support for HTTPS over TLS 1.2, which is crucial to connecting to most secure websites or API endpoints.

### (Extended) WPA2-Enterprise authentication with username/password

Add support for WPA2-Enterprise authentication with username and password combination (always assume certificate is valid). Important for supporting most school Wi-Fi networks that are enterprise networks, not PSK. Most school networks employ the following connection parameters:

- WPA-EAP Key management
- PEAP as Phase 1
- MSCHAPv2 as Phase 2

### (Extended) HTTP/REST handling for very large response strings

As the master device such as an ATmega328P may not be able to handle overly large strings due to its limited RAM, additional processing and heavy lifting such as JSON parsing should be offloaded to the ESP8266 and only return relevant data.

## Basic environment setup

As this repository is a locally built repository, you must have Docker installed (for Mac, simply download, install, and start Docker from the official website). Docker must be configured to share this folder with Docker images so the build toolchain has permission to read and write in this folder.

Make sure that you have built the firmware at least once for the `mos` tool to automatically download the Mongoose OS repository into `deps/modules` to allow IntelliSense to detect the headers.

For optimal experience, install the Mongoose OS IDE extension in Visual Studio Code to configure the default serial port without having to manually set environment variables.

## Caveats

WIP
