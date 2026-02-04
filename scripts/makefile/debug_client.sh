#!/bin/bash
# Color Definitions
CYAN='\033[1;36m'
NC='\033[0m'

CORE=${1:-cm7}
PORT=3333
[ "$CORE" == "cm4" ] && PORT=3334

echo -e "${CYAN}Attaching GDB to $CORE on port $PORT...${NC}"
arm-none-eabi-gdb -ex "target extended-remote :$PORT" build/$CORE.elf
