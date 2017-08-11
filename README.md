# IEC61850-Server

![User interface #1](images/running-server.png)
![User interface #2](images/stopped-server.png)

## How to use


## Building
### Dependencies
- libiec61850 (_included in the source_)
- OpenGL (_should be included in a standard Linux distro_)
- SDL2
- GLEW
- Nuklear (_included in the source_)

*Install the dependencies*
```bash  
$ sudo apt-get install libsdl2-dev libglew-dev
```
then change in to the directory of the repository and ...
*Run*
```bash  
$ make && sudo ./main <network-interface-name>
```
The first and only command line argument is the network interface name which
is typically something along the lines of 'eth0' on Linux. It defaults to 'eth0'
on all platforms.

The program needs to be run as sudo due to the low-level network access required
by libiec61850.

### Performance concerns
Given that the number of samples sent over the network quite easily can move into the thousands per seconds (50 hz * 256 samples/s = 12800 values/s) the performance of the program needs to be investigated. The normal build is unoptimized (add '-O3' to CXXFLAGS variable (line ~15) in the makefile for optimized builds).

Tested on Ubuntu 17.04.
