/**
 * @file common.h
 * @brief Umbrella header for shared project components.
 *
 * This header aggregates hardware-specific definitions and utility modules.
 * It is designed to be used as a Precompiled Header (PCH) to significantly
 * reduce compilation times across the dual-core build environment.
 * * @author LM
 */

#ifndef COMMON_H
#define COMMON_H

#include "stm32h755.h"
#include "uart.h"
#include "utils.h"
/**
 * @def PROJECT_NAME
 * @brief The formal name of the firmware project.
 */
#define PROJECT_NAME "MakeyDooey"

#endif /* COMMON_H */
