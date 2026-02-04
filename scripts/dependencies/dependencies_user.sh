#!/bin/bash
source $(dirname "$0")/dependencies_core.sh

echo "Installing Local Dev Environment..."
sudo apt-get update
sudo apt-get install -y $CORE_PACKAGES gcc-arm-none-eabi binutils-arm-none-eabi libnewlib-arm-none-eabi

# Optional: Set up the git hook we discussed earlier
git config core.hooksPath scripts
echo "User setup complete. Git hooks activated."
