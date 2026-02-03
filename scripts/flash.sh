#!/bin/bash
# Color Definitions
GREEN='\033[1;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

CORES=$(nproc)
echo -e "${YELLOW}Building and Flashing (using $CORES threads)...${NC}"

# Run the build
make -j$CORES all || exit 1

# Run the flash
openocd -f openocd.cfg \
  -c "init" -c "reset halt" \
  -c "program build/cm7.bin 0x08000000 verify" \
  -c "program build/cm4.bin 0x08100000 verify" \
  -c "reset run" -c "exit" >/dev/null 2>&1

echo -e "${GREEN}Flash Complete.${NC}"
