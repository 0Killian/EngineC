/**
 * @file math.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This file defines common math utilities.
 * @version 0.1
 * @date 2024-07-19
 */

#include "common.h"
#include_next <math.h>

/** @brief Align an integer upwards to the next multiple of a */
#define ALIGN_UP(x, a) (((x) + ((a) - 1)) & ~((a) - 1))

/** @brief Align an integer downwards to the next multiple of a */
#define ALIGN_DOWN(x, a) ((x) & ~((a) - 1))

/** @brief Return the minimum of two integers */
#define MIN(x, y) ((x) < (y) ? (x) : (y))

/** @brief Return the maximum of two integers */
#define MAX(x, y) ((x) > (y) ? (x) : (y))

/** @brief Return the clamped value */
#define CLAMP(x, min, max) MAX(min, MIN(x, max))

static inline f32 pow32(f32 x, f32 y) {
    return powf(x, y);
}
