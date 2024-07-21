/**
 * @file math.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This file defines common math utilities.
 * @version 0.1
 * @date 2024-07-19
 */

/** @brief Align an integer upwards to the next multiple of a */
#define ALIGN_UP(x, a) (((x) + ((a) - 1)) & ~((a) - 1))
