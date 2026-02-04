/**
 * @file utils.h
 * @brief Common utility functions and low-level hardware macros.
 *
 * Provides essential helper functions for timing and cache management
 * that are utilized by both the Cortex-M7 and Cortex-M4 cores.
 */

#ifndef UTILS_H
#define UTILS_H

/**
 * @def SCB_DCCMVAC
 * @brief Data Cache Clean by MVA to PoC (Point of Coherency).
 * * Pointer to the System Control Block register used to force a cache clean
 * for a specific address, ensuring memory consistency between cores or DMA.
 */
#define SCB_DCCMVAC (*(volatile unsigned int *)0xE000EF68)

/**
 * @brief Simple busy-wait delay loop.
 * * @param count Number of iterations to perform.
 * Marked volatile to prevent compiler optimization.
 */
void delay(volatile int count);

/**
 * @brief Flushes the data cache for a specific memory address.
 * * Uses the SCB_DCCMVAC register to ensure data is written back to
 * the main memory (SRAM) so it is visible to other bus masters.
 * * @param addr The start address of the memory block to flush.
 */
void flush_cache_addr(unsigned int addr);

#endif /* UTILS_H */
