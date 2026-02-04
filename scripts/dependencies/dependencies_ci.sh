#!/bin/bash
# Use dirname to ensure the script works even if called from the root folder
source "$(dirname "$0")/dependencies_core.sh"

echo "Setting up CI Environment..."
sudo apt-get update
sudo apt-get install -y $CORE_PACKAGES
