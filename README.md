# MakeyDooey — Firmware Repository

Firmware for the [MakeyDooey](https://github.com/MakeyDooey) system.

## Overview
This repository contains the firmware that runs on the MakeyDooey embedded platform. The project targets a heterogeneous MCU setup (Cortex‑M7 and Cortex‑M4 cores) and provides the low-level software, board bring‑up, and runtime environment required by the MakeyDooey hardware.

## Progress
### 2026-01-26 to 2026-02-06 (feature 1)
Current sprint highlights:
- Docker: Added a project Docker environment to standardize builds and toolchains.
- CI/CD: Pipeline work in progress to automate builds and tests (integration with repository CI).
- OpenOCD: OpenOCD configuration added/updated for flashing and debugging the boards.
- FreeRTOS on M7: FreeRTOS is running on the M7 core (early integration complete).
- Bare‑metal on M4: M4 core running bare‑metal firmware with a custom UART driver implemented.

These items represent the main milestones achieved during the sprint; there is ongoing work to stabilize CI/CD and extend test coverage (see [feature-1-sprint](https://github.com/MakeyDooey/Firmware/tree/feature-1-sprint) for detailed breakdown of progress made).

## Getting started
1. Install toolchain and dependencies (see [dependencies.md](https://github.com/MakeyDooey/Firmware/blob/main/dependencies.md)).
2. Use the Docker environment to build and run the firmware to ensure consistent tool versions: ```make docker-build```
3. Flash with OpenOCD: ```make flash```
    - see [Makefile](https://github.com/MakeyDooey/Firmware/blob/main/Makefile) for more information on how Docker + OpenOCD are configure for this hardware 
4. Boot sequence:
   - M7: FreeRTOS-based firmware image
   - M4: Bare-metal image (with custom UART driver)
