# IEC61850-Server

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
```bash  
$ make && sudo ./main
```
The program needs to be run as sudo due to the low-level network access required
by libiec61850.

Tested on Ubuntu 17.04.
