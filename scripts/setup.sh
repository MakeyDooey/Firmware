#!/bin/bash

# Define required packages for Debian/Ubuntu
DEPS="gcc-arm-none-eabi binutils-arm-none-eabi libnewlib-arm-none-eabi build-essential openocd python3 gdb-multiarch"

echo "Checking dependencies..."

# Update package lists first to ensure installs don't fail
sudo apt update -y

# Loop through and install missing tools
for tool in arm-none-eabi-gcc make openocd python3 gdb-multiarch; do
  if ! command -v $tool &>/dev/null; then
    echo "$tool missing. Installing dependencies..."
    sudo apt install -y $DEPS
    break
  else
    echo "$tool found."
  fi
done

echo "Environment check complete."
