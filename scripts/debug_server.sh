#!/bin/bash
# Color Definitions
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${YELLOW}Starting OpenOCD GDB Server...${NC}"
openocd -f openocd.cfg
