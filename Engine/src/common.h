/**
 * @file common.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This file defines common data types and macros used throughout the project.
 * @version 1.0
 * @date 2024-06-11
 */

#pragma once

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef float f32;
typedef double f64;

typedef u8 b8;
#define FALSE 0
#define TRUE 1

#ifdef _MSC_VER
    #ifdef EXPORT
        #define API __declspec(dllexport)
    #else
        #define API __declspec(dllimport)
    #endif
#else
    #ifdef EXPORT
        #define API __attribute__((visibility("default")))
    #else
        #define API
    #endif
#endif