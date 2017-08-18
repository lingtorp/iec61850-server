#!/bin/bash
# Launch the server
cd ~/Desktop/iec61850-server
make && gksu ./main lo
