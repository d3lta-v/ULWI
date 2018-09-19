# Instruction Set Architecture

This document provides an outline of the instruction set, version 1.0-alpha1. All alpha releases are **SUBJECT TO CHANGE** and the interface should not be considered stable!

If you are confused about what this instruction set is supposed to do, please read the original [README](README.md) document.

All operations are terminated with a Windows style line ending, `\r\n`. To process messages involving multiple lines, please use UNIX style line endings (`\n`).

The instruction set is inspired by CISC instruction sets such as x86, in order to provide as much functionality as possible, making programming easier for the master side.

Any instructions that are synchronous in nature is explicitly marked so. All attempts have been made to make the program flow as asynchronous as possible, but there are still certain instructions that are fundamentally synchronous and requires waiting.

Commands can be broadly separated into 3 types: action, reply or blocking reply.

- Action: This type of command performs an action. The ESP8266 will only reply with a Windows-style newline. For example, initiating a HTTP request is an action command and does not return anything.
- Reply: This type of command will result in a reply by the ESP8266. For example, checking on the status of a HTTP request will always incur a response.
- Blocking reply: This is a hybrid of Action and Reply and should be used sparingly. This type of commands are typically not implemented and should be avoided.

## Initialisms

This instruction set uses extensive amounts of letters to represent certain settings. The following list is a common set of letters used across all instructions. Instruction specific letters would be included in the manual entry for that particular instruction

General booleans (describes general true or false statements):

- T: True
- F: False

Task status (describes the status of a task, such as connection to an AP or a HTTP request):

- S: Successful
- U: Unsuccessful
- P: In Progress (the task is being processed right now)
- N: Not attempted (the task was never started in the first place)

HTTP method types:

- G: GET
- P: POST

## Basic operations

### No Operation

Command: `nop`  
Type: Action  
Purpose: This command performs virtually nothing except forces the module to echo back a blank response consisting of a Windows-style newline (`\r\n`). Useful for testing if the module is functioning correctly or that the software is still responding.

### Display VERsion

Command: `ver`  
Type: Reply  
Purpose: Outputs the firmware's version over the serial link. Useful for checking if the current firmware supports certain commands.  
Output: `1.0-alpha1,Mongoose 2.5.1`

### ReSeT

Command: `rst`  
Type: Blocking Reply  
Purpose: Performs a software reset of the module.

## Wi-Fi operations

### List Access Points

Command: `lap`  
Type: Blocking Reply  
Purpose: Lists all available access points in the vicinity. Comma delimited. This function may be *blocking*, meaning that it may take a significant amount of time to run.  
Returns: `WiFi1,Wi-Fi Hotspot 2,AnotherWifi`

### Connect to Access Point

Command: `cap <ssid>,<password>`  
Type: Action  
Purpose: Connects to an access point with SSID and password. This method does not pass back a connection success or failed message. To query if the connection was successful, use `sap` instead.  
Parameters:

- `<ssid>`: The SSID of the Wi-Fi access point to connect to
- `<password>`: The PSK of the Wi-Fi access point to connect to

### Status of Access Point

Command: `sap`  
Type: Reply  
Purpose: Checks if the ESP8266 is properly connected to an access point.  
Returns: `<S/U/P/N>,<ssid (only if connection is successful)>` based on the connectivity state.

### Disconnect from Access Point

Command: `dap`  
Type: Action  
Purpose: Disconnects from the currently connected access point.

## IP and DHCP operations

### Set DHCP enabled

Command: `sde <T/F>`  
Type: Action  
Purpose: Sets whether should DHCP be enabled  
Parameters:

- `<T/F>`: True or False, whether DHCP should be enabled

### Get IP

Command: `gip`  
Type: Blocking Reply  
Purpose: Gets the current IP address of the module, when connected to Wi-Fi  
Returns: `0` if there is no IP assigned, IP address if there is an IP

### Set IP

Command: `sip <ip>,<gateway>,<netmask>`  
Type: Action  
Purpose: Sets the IP address, gateway and netmask of the ESP8266, if we're using static IP. DHCP must be disabled prior to setting an IP address manually  
Parameters:

- `<ip>`: The IP address to set
- `<gateway>`: The gateway to set
- `<netmask>`: The network mask to set

## HTTP operations

### Initiate HTTP Request

Command: `ihr <G/P>,<url>,<content>,<port (optional)>`  
Type: Action  
Purpose: Initiates a HTTP request to a URL. This method does not report back on the progress or completion of the request.  
Parameters:

- `<G/P>`: GET or POST
- `<url>`: The full URL, including protocol. Example: `https://fourier.industries/endpoint`
- `<content>`: The content to send
- `<port (optional)>`: The port to use. Defaults to 80 if protocol is HTTP and 443 if HTTPS

Returns: A unique identifier or handle that identifies the HTTP request

### Status of HTTP Request

Command: `shr <http request handle>`  
Type: Reply  
Purpose: Checks on the status of a HTTP request.  
Parameters:

- `<http request handle>`: The HTTP request handle issued to you by the `ihr` command

Returns: `<S/U/P/N>` based on the status of the connection. S -> Success, U -> Unsuccessful, P -> In Progress, N -> No such HTTP request handle

### Get HTTP Response

Command: `ghr <http request handle>,<S/H/C>,<T/F>`  
Type: Reply/Blocking Reply  
Purpose: Gets the result of a HTTP request. The request must be completed before attempting to retrieve it.  
Parameters:

- `<http request handle>`: The HTTP request handle issued to you by the `ihr` command
- `<S/H/C>`: Status, Headers, Content
- `<T/F>`: True to delete the result, False to keep the result in the ESP8266

Returns: WIP

### Get HTTP Response Content as JSON

Command: `jhr <http request handle>,<json map>`  
Type: Reply/Blocking Reply  
Purpose: Gets parts of the HTTP response as JSON  
Parameters:

- `<http request handle>`: The HTTP request handle issued to you by the `ihr` command
- `<json map>`: WIP

### Purge HTTP Response

Command: `phr <http request handle>`  
Type: Reply  
Purpose: Frees the RAM taken by a HTTP response on the ESP8266 while freeing up the handle as well to be used by other HTTP requests  
Parameters:

- `<http request handle>`: The HTTP request handle issued to you by the `ihr` command

### Terminate HTTP request

Command: `thr <http request handle>`  
Type: Action  
Purpose: Terminates a HTTP request.  
Parameters:

- `<http request handle>`: The HTTP request handle issued to you by the `ihr` command
