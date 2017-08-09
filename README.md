# Building
## Dependencies
- OpenGL (should be included in a standard Linux distro)
- SDL2
- GLEW
- Nuklear (included in the source)

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
