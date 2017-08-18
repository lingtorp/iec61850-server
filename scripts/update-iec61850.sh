#!/bin/bash
# Update the client
cd ~/Desktop/iec61850-client/
git pull
make

# Update the server
cd ..
cd iec61850-server
git pull
make
