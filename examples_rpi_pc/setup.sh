#!/bin/bash

# Install necessary software
if [ -f "/etc/debian_version" ]; then
    echo " -- Installing necessary libraries and tools --"
    sudo apt-get install git libmosquittopp1 libmosquittopp-dev
fi
grep -qs Raspberry /proc/device-tree/model
if [ $? -eq 0 ]; then
    echo " -- Installing wiringpi for raspberry --"
    sudo apt-get install wiringpi
fi

# Create a folder in home (assuming user is pi)
echo " -- Creating folder --"
cd ~
mkdir hc-mqtt
cd hc-mqtt

# Clone the git repo
echo " -- Cloning the git repository --"
git clone https://github.com/HomeControlAS/homecontrol-mqtt.git --depth=1 --recurse-submodules -j4 homecontrol-mqtt

# Copy necessary files
echo " -- Copying files for building --"
cp homecontrol-mqtt/examples_rpi_pc/$FOLDER/Makefile .
cp homecontrol-mqtt/examples_rpi_pc/$FOLDER/main.cpp .
echo " -- Now run 'make' to build the libraries and the test software --"
