# Instruction Set Architecture

This document provides an outline of the instruction set, version 1.0-alpha1. All alpha releases are **SUBJECT TO CHANGE** and the interface should not be considered stable!

If you are confused about what this instruction set is supposed to do, please read the original [README](README.md) document.

All commands and replies are terminated with a Windows style line ending, `\r\n` unless otherwise noted. To work with messages involving multiple lines, please use UNIX style line endings (`\n`).

In addition, all parameters are delimited with the ASCII Unit Separator control character (0x1F) to avoid excessive use of escape characters or sequences. **This character is represented by the vertical bar character (|) in the rest of this manual.**

The instruction set is inspired by CISC instruction sets such as x86, in order to provide as much functionality as possible, making programming easier for the master side.

Any instructions that are synchronous in nature is explicitly marked so. All attempts have been made to make the program flow as asynchronous as possible, but there are still certain instructions that are fundamentally synchronous and requires waiting.

Commands can be broadly separated into 3 types: action, reply or blocking reply.

- Action: This type of command performs an action. The ESP8266 will only reply with a Windows-style newline. For example, initiating a HTTP request is an action command and does not return anything.
- Reply: This type of command will result in a reply by the ESP8266. For example, checking on the status of a HTTP request will always incur a response.
- Blocking reply: This is a hybrid of Action and Reply and should be used sparingly. This type of commands are typically not implemented and should be avoided.

## Initialisms

This instruction set uses extensive amounts of letters to represent certain settings. The following list is a common set of letters used across all instructions. Instruction specific letters would be included in the manual entry for that particular instruction

General booleans (describes general true or false statements):

- **T**: True
- **F**: False

Task status (describes the status of a task, such as connection to an AP or a HTTP request):

- **S**: Successful
- **U**: Unsuccessful
- **P**: In Progress (the task is being processed right now)
- **N**: Not attempted (the task was never started in the first place)

HTTP method types:

- **G**: GET
- **P**: POST

## Basic operations

### No Operation

**Command**: `nop`  
**Type**: Action  
**Purpose**: This command performs virtually nothing except forces the module to echo back a blank response consisting of a Windows-style newline (`\r\n`). Useful for testing if the module is functioning correctly or that the software is still responding.

### Display VERsion

**Command**: `ver`  
**Type**: Reply  
**Purpose**: Outputs the firmware's version over the serial link. Useful for checking if the current firmware supports certain commands.  
**Output**: `1.0-alpha1`

### ReSeT

**Command**: `rst`  
**Type**: Blocking Reply  
**Purpose**: Performs a software reset of the module.

## Wi-Fi operations

### List Access Points

**Command**: `lap`  
**Type**: Blocking Reply  
**Purpose**: Lists all available access points in the vicinity. Comma delimited. This function may be *blocking*, meaning that it may take a significant amount of time to run.  
**Returns**: `WiFi1|Wi-Fi Hotspot 2|AnotherWifi`

### Connect to Access Point

**Command**: `cap <ssid>|<password>`, `cap <ssid>|<password>(|<ip>|<gw>|<netmask>)` or `cap <ssid>|<username>|<password>`  
**Type**: Action  
**Purpose**: Connects to an access point. If there are only 2 arguments, the OS will assume authentication to a WPA/WPA2-PSK network. If there are 5 arguments, the OS WILL assume authentication to a WPA/WPA2-PSK network with static IP. If there are 3 arguments, the OS will assume authentication to a WPA2-Enterprise network with PEAP authentication. This method does not pass back a connection success or failed message. To query if the connection was successful, use `sap` instead.  
**Parameters**:

- `<ssid>`: The SSID of the Wi-Fi access point to connect to
- `<username>` (optional): The username of the user for authenticating to enterprise networks
- `<password>`: The password of the Wi-Fi access point to connect to
- `<ip>` (optional): The desired IP address of the current device
- `<gw>` (optional): The gateway IP of the network
- `<netmask>` (optional): The net mask of the network

### Status of Access Point

**Command**: `sap`  
**Type**: Reply  
**Purpose**: Checks if the ESP8266 is properly connected to an access point.  
**Returns**: `<S/U/P/N>,<ssid (only if connection is successful)>` based on the connectivity state.

### Disconnect from Access Point

**Command**: `dap`  
**Type**: Action  
**Purpose**: Disconnects from the currently connected access point.

## IP and DHCP operations

### Set DHCP enabled

**Command**: `sde <T/F>`  
**Type**: Action  
**Purpose**: Sets whether should DHCP be enabled  
**Parameters**:

- `<T/F>`: True or False, whether DHCP should be enabled

### Get IP

**Command**: `gip`  
**Type**: Blocking Reply  
**Purpose**: Gets the current IP address of the module, when connected to Wi-Fi  
**Returns**: `0` if there is no IP assigned, IP address if there is an IP

## HTTP operations

### Initialise HTTP Request

**Command**: `ihr <G/P>|<url>`  
**Type**: Action  
**Purpose**: Initialises a HTTP request to a URL with all of the required parameters, and creates a unique identifier. This command does not actually send the HTTP request.  
**Parameters**:

- `<G/P>`: GET or POST
- `<url>`: The full URL, including protocol (e.g. HTTP or HTTPS). Example: `https://fourier.industries/endpoint/`

**Returns**: A unique identifier or handle that identifies the HTTP request, or `U` if it failed to create the handle

### POST parameters of HTTP Request

**Command**: `phr <http request handle>|<parameters>`  
**Type**: Action  
**Purpose**: Specifies the POST body of a HTTP request before sending the request. This is done separately from `ihr`.  
**Parameters**:

- `<http request handle>`: The HTTP request handle issued to you by the `ihr` command
- `<parameters>`: The POST data of the HTTP request. Example: `var_1=value1&var_1=value2`, which is typically behind the URL. Keep in mind that if you are POSTing form-encoded data (similar to the example above) that you have to specify a header with the value "Content-Type: application/x-www-form-urlencoded\n" by using the `hhr` command.

**Returns**: `<S/U>` Successful or Unsuccessful. Returns `U` if that HTTP request handle does not exist, or that the HTTP handle is a GET request and does not support this field.

### Headers of HTTP Request

**Command**: `hhr <http request handle>|<headers>`  
**Type**: Action  
**Purpose**: Specifies the contents field of a HTTP request before sending the request. This is done separately from `ihr`.
**Parameters**:

- `<http request handle>`: The HTTP request handle issued to you by the `ihr` command
- `<headers>`: The headers of the HTTP request. Example: `Header1: value1\nHeader2: value2\n`. This method ONLY accepts UNIX style line endings to separate headers (e.g. LF) and not Windows style line endings actually used by web servers (e.g. CRLF) to prevent the data from interfering with the command.

**Returns**: `<S/U>` Successful or Unsuccessful. Returns `U` if that HTTP request handle does not exist.

### Transmit HTTP Request

**Command**: `thr <http request handle>`  
**Type**: Action  
**Purpose**: Transmits a HTTP request to the server specified.  
**Parameters**:

- `<http request handle>`: The HTTP request handle issued to you by the `ihr` command

**Returns**: `<S/U>` Successful or Unsuccessful. Returns `U` if that HTTP request handle does not exist.

### Status of HTTP Request

**Command**: `shr <http request handle>`  
**Type**: Reply  
**Purpose**: Checks on the status of a HTTP request. This is different from the HTTP status code (e.g. 200, 404, 500), which will be reflected by the `ghr` command.  
**Parameters**:

- `<http request handle>`: The HTTP request handle issued to you by the `ihr` command

**Returns**: `<S/U/P/N>` based on the status of the connection. S -> Success, U -> Unsuccessful, P -> In Progress, N -> No such HTTP request handle

### Get HTTP Response

**Command**: `ghr <http request handle>|<S/H/C>|<T/F>`  
**Type**: Reply/Blocking Reply  
**Purpose**: Gets the result of a HTTP request. The request must be completed before attempting to retrieve it.  
**Parameters**:

- `<http request handle>`: The HTTP request handle issued to you by the `ihr` command
- `<S/H/C>`: Status, Headers, Content
- `<T/F>`: True to delete the result, False to keep the result in the ESP8266

**Returns**: Replies with "U" if the HTTP request handle is invalid or is not available for reading. Replies with the status (e.g. `200`) if second parameter is `S`, replies with all the headers if second parameter is `H` and replies with the content of the response if second parameter is `C`.

### Get HTTP Response Content as JSON

**Command**: `jhr <http request handle>|<json map>`  
**Type**: Reply/Blocking Reply  
**Purpose**: Gets parts of the HTTP response as JSON  
**Parameters**:

- `<http request handle>`: The HTTP request handle issued to you by the `ihr` command
- `<json map>`: WIP

**Returns**: WIP

### Delete HTTP Response

**Command**: `dhr <http request handle>`  
**Type**: Reply  
**Purpose**: Frees the RAM taken by a HTTP response on the ESP8266 while freeing up the handle as well to be used by other HTTP requests  
**Parameters**:

- `<http request handle>`: The HTTP request handle issued to you by the `ihr` command

**Returns**: `\r\n` if command was executed successfully, `U` if the command failed (such as when there is no such handle or this handle is already empty)

## MQTT Operations

### MQTT Configure

**Command**: `mcg <T/F>|<server:port>|<T/F>(|<username>|<password>)`  
**Type**: Reply  
**Purpose**: Configures MQTT client with parameters    
**Parameters**:

- `<T/F>`: Boolean which enables or disables the MQTT client
- `<server:port>`: Server and port of the MQTT broker/server
- `<T/F>`: Boolean which enables or disables TLS. By default, ULWI uses a predefined list of certificates from major certficate authorities to verify the authenticity of0 the site. If one wishes to alter the encryption settings, refer to the injection commands below
- `<username>` (Optional): Username for authenticating to the MQTT server
- `<password>` (Optional): Password for authenticating to the MQTT server

**Returns**: `\r\n` if command was executed successfully, `U` if the command failed (such as when there is no such handle or this handle is already empty)

### MQTT Is Connected

**Command**: `mic`  
**Type**: Reply  
**Purpose**: Checks if the MQTT client is connected.  
**Parameters**: None  
**Returns**: `T` or `F` depending on if the MQTT client is connected.

### MQTT Inject Certificate Authority Certificate

**Command**: `mca <certificate>`  
WIP. This feature is mainly meant for higher level X.509 based authentication such as with AWS IoT.

### MQTT Inject Client Certificate

**Command**: `mcc <certificate>`  
WIP. This feature is mainly meant for higher level X.509 based authentication such as with AWS IoT.

### MQTT Inject Private Key

**Command**: `mpk <key>`  
WIP. This feature is mainly meant for higher level X.509 based authentication such as with AWS IoT.

### MQTT Subscribe

**Command**: `msb <topic>`  
**Type**: Action  
**Purpose**: Subscribes to an MQTT topic  
**Parameters**: 

- `<topic>`: The MQTT topic to subscribe to

**Returns**: `\r\n` (Windows style newline)

### MQTT Check for New Data Arrival

**Command**: `mnd`  
**Type**: Reply  
**Purpose**: Checks if a new piece of data has just arrived from a subcription  
**Parameters**: None  
**Returns**: `<T/F>\r\n` boolean value depending on whether new data has arrived. True values will turn to False after `mgs` has been run to mark the data as stale.


### MQTT Get Subscribed Data

**Command**: `mgs`  
**Type**: Reply  
**Purpose**: Retrieves the latest data. This resets the `mnd` command's reply from true to false.  
**Parameters**: None  
**Returns**: The raw data which was retrieved after subscription

### MQTT Publish

**Command**: `mpb <topic>|<content>|<qos>|<T/F>`  
**Type**: Reply  
**Purpose**: Publishes to MQTT broker  
**Parameters**:

- `<topic>`: The MQTT topic to publish to
- `<content>`: The content of the MQTT message
- `<qos>`: Quality of Service level, either 0 (no guarantee of delivery) or 1 (guaranteed delivery)
- `<T/F>`: Boolean that sets the retain flag on the MQTT message. Retained messages allows new MQTT subscribers to 

**Returns**: `\r\n` if command was executed successfully, `U` if the command failed (such as when the MQTT client is not active)
