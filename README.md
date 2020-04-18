# Compression Detection

A project where there are two network applications that work together to detect whether network compression is present.\
One application is a client/server application and the other application is a standalone application which works in an uncooperative environment.\
That is, the application detects network compression without requiring a special software being installed on the other host.\
This is inspired by the work:\
[End-to-End Detection of Compression of Traffic Flows by Intermediaries ](https://lasr.cs.ucla.edu/vahab/resources/compression_detection.pdf)


## Installation
**This program also requires you to download JSON. To do that on a linux computer input the following command into your terminal.**\
```bash
sudo apt install libjson-c-dev
#Now to install the repository
git clone http://github.com/dennisdang17/Compression-Detection.git
```
## Usage
**Client Server Application**
1) Change the JSON file to the correct parameters you wish to use.\
The Config file contains :
```
    1. The Server’s IP Address
    2. Source Port Number for UDP
    3. Destination Port Number for UDP
    4. Destination Port Number for TCP Head SYN, x
    5. Destination Port Number for TCP Tail SYN, y
    6. Port Number for TCP (Pre-/Post- Probing Phases)
    7. The Size of the UDP Payload in the UDP Packet Train, (default value: 1000B)
    8. Inter-Measurement Time, γ (default value: 15 seconds)
    9. The Number of UDP Packets in the UDP Packet Train, n (default value: 6000 )
    10. TTL for the UDP Packets (default value: 255 )
```
2) Compile the source code with the commands below\
```c
//Compile on server machine
gcc server.c -ljson-c -Wall -o "name of executable"

//Compile on client machine
gcc client.c -ljson-c -Wall -o "name of executable"
```
3) Run the application \
**MAKE SURE TO RUN APPLCIATION ON SERVER SIDE FIRST**
```
#On the server system
./"name of executable" myconfig.json
#On the client system
./"name of executable" myconfig.json
```
