#!/bin/bash
# Launch the client
cd ~/Desktop/iec61850-client/
make && gksu ./main lo
