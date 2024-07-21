/**
 * @file vec2.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This file defines 2-component vectors and functions for them.
 * @version 0.1
 * @date 2024-07-19
 */

#pragma once

#include "common.h"
#include <xmmintrin.h>

/** @brief 2D vector of f32. */
typedef union vec2f {
    struct {
        f32 x;
        f32 y;
    };
    __m128 _mmv;
} __attribute__((aligned(16))) vec2f;

/**
 * @brief Checks if the two vectors are equal.
 * 
 * @param[in] a The first vector.
 * @param[in] b The second vector.
 * 
 * @retval TRUE The vectors are equal.
 * @retval FALSE The vectors are not equal.
 */
static inline b8 vec2f_equals(vec2f a, vec2f b) {
    return (((_mm_movemask_ps(_mm_cmpeq_ps(a._mmv, b._mmv))) & 0x3) == 0x3) ? TRUE : FALSE;
}

/**
 * @brief Subtracts the second vector from the first.
 * 
 * @param[in] a The first vector.
 * @param[in] b The second vector.
 * 
 * @return The difference between the two vectors.
 */
static inline vec2f vec2f_sub(vec2f a, vec2f b) {
    return (vec2f) { ._mmv = _mm_sub_ps(a._mmv, b._mmv) };
}