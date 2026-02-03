#!/bin/bash
# Color Definitions
RED='\033[1;31m'
NC='\033[0m'

echo -e "${RED}MASS ERASING BOTH FLASH BANKS...${NC}"

openocd -f openocd.cfg -c "init" -c "reset halt" \
  -c "stm32h7x mass_erase 0" -c "stm32h7x mass_erase 1" -c "exit" >/dev/null 2>&1

echo -e "${RED}Flash Cleaned.${NC}"
