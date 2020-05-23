# Compression Detection

A project where there are two network applications that work together to detect whether network compression is present.\
One application is a client/server application and the other application is a standalone application which works in an uncooperative environment.\
That is, the application detects network compression without requiring a special software being installed on the other host.\
This is inspired by the work:\
[End-to-End Detection of Compression of Traffic Flows by Intermediaries ](https://lasr.cs.ucla.edu/vahab/resources/compression_detection.pdf)

## Requirements
**YOU MUST HAVE A LINUX MACHINE IN ORDER TO BE ABLE TO RUN THIS PROGRAM. JSON ALSO MUST BE INSTALLED. PLEASE REFER TO INSTALLATION IF YOU HAVE PROBLEMS INSTALLING JSON**

## Installation
**This program also requires you to download JSON. To install both JSON and the appllication on your linux computer/machine input the following commands into your terminal.**
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
## Project Status / Known Issues
The server - client application is complete. \
The program has a bug where sometimes the packets will get dropped sometime on the way to the server, this leads to the server to continue to wait for the total number of packets to arive even though that will never happen as a packet is lost. Therefore the program crashes and that is an issue. A couple fixes we have come up with are timeouts after not receiving the total amount of packets, or to just send small packet trains as the chances of packets getting lost then are hard. 

The standalone program is still a work in progress. Our plan was to try to run the example of a raw socket implementation from the CS336 website so that we could observe how the program worked and from there we would figure out a plan to begin our implementation. However, we could not run the program. We added the JSON parsing and the sending of packets implementation.
## Contributors
Dennis Dang - [Github](https://github.com/dennisdang17) | [Linkedin](https://www.linkedin.com/in/dennisqdang) \
Gerardo Aldana - [Github](https://github.com/GeoDude1)

## License
<a rel="license" href="http://creativecommons.org/licenses/by/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by/4.0/80x15.png" /></a><br />This work is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by/4.0/">Creative Commons Attribution 4.0 International License</a>.
