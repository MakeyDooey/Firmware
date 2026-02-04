# **📊 Project Dependencies & Setup**

This repository contains a dual-core firmware project for the **STM32H755**. To ensure build consistency between your local machine and the GitHub CI/CD pipeline, we use a modular dependency management system located in the scripts/ directory.

## **🚀 Quick Start (Automated Setup)**

If you are on a Debian-based Linux distribution (Ubuntu/Debian/Mint), you can set up your entire environment with a single command:

chmod \+x scripts/dependencies/dependencies\_user.sh  
./scripts/dependencies/dependencies\_user.sh

**What this script does:**

1. **System Update:** Synchronizes your local package indexes.  
2. **Toolchain:** Installs gcc-arm-none-eabi, binutils, and newlib.  
3. **Build Tools:** Installs make and ccache for high-performance builds.  
4. **Quality Gates:** Installs cppcheck (static analysis) and clang-format (styling).  
5. **Hardware:** Installs openocd and gdb-multiarch for flashing/debugging.  
6. **Automation:** Configures your local Git hooks to point to scripts/ to catch errors before you push.

## **🛠 Manual Toolchain Requirements**

If you are using a different OS (macOS/Windows) or prefer manual installation, ensure these tools are in your system $PATH:

| Tool | Purpose | Source/Command |
| :---- | :---- | :---- |
| arm-none-eabi-gcc | Cross-compiler for Cortex-M7/M4 | [ARM Developer](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) |
| make | Build automation | sudo apt install make |
| ccache | Compiler object caching | sudo apt install ccache |
| cppcheck | Static analysis bug hunting | sudo apt install cppcheck |
| clang-format | Code style enforcement | sudo apt install clang-format |
| openocd | JTAG/SWD Flashing tool | sudo apt install openocd |

## **📁 Script Architecture**

We use a "Core \+ Environment" architecture to keep our CI and User environments identical.

* **scripts/dependencies/dependencies\_core.sh**: The master list of shared packages.  
* **scripts/dependencies/dependencies\_user.sh**: Local setup (includes ARM compiler \+ Git Hooks).  
* **scripts/dependencies/dependencies\_ci.sh**: Optimized for GitHub Actions (excludes local toolchain to use the Action-provided one).

## **🎨 Quality Control**

### **1\. Formatting**

We follow the rules defined in .clang-format. To format your code locally before pushing:

find common/ cm7/ cm4/ \-name "\*.c" \-o \-name "\*.h" | xargs clang-format \-i

### **2\. Static Analysis**

The CI runs cppcheck on every push. You can run it locally to see warnings:

cppcheck \--enable=warning,performance,portability common/ cm7/ cm4/

## **🔌 Hardware Debugging**

For the **Nucleo-H755ZI-Q**, use the integrated helper scripts mapped to the root Makefile:

* make flash: Calls scripts/makefile/flash.sh to program the board.  
* make wipe: Calls scripts/makefile/wipe.sh to erase the flash sectors.  
* make debug: Starts an OpenOCD server and GDB client session.
