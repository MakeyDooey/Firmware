#!/bin/bash

# Add or remove packages here - one per line
PACKAGES="
gcc-arm-none-eabi
binutils-arm-none-eabi
libnewlib-arm-none-eabi
build-essential
openocd
gdb-multiarch
ccache
"

echo "Updating system..."
sudo apt update -y

echo "Installing dependencies..."
sudo apt install -y $PACKAGES

echo "Check complete."
