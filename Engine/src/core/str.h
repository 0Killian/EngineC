#pragma once

#include "common.h"

/**
 * @brief Creates the argument for printing a string view
 * Example usage: LOG_INFO("%.*s", STR_VIEW_PRINT(view));
 *
 * @param[in] view The string view to print
 */
#define STR_VIEW_PRINT(view) (view).size, (view).begin

/** @brief A view to a part of a string */
typedef struct str_view {
    const char *begin;
    u32 size;
} str_view;

/** @brief Type of trimming to apply */
typedef enum trim_type {
    TRIM_LEFT = 0x1,
    TRIM_RIGHT = 0x2,
    TRIM_BOTH = TRIM_LEFT | TRIM_RIGHT
} trim_type;

/**
 * @brief Creates a string view from a C string
 *
 * @note The string is not duplicated, so any modification of this view will affect the original, and no free operation is needed
 *
 * @param[in] cstr The C string
 *
 * @returns The string view
 */
str_view str_view_from_cstr(const char *cstr);

/**
 * @brief Splits a string view by the provided delimiters into a subpart and the remaining
 * 
 * @param[in,out] str The string view
 * @param[in] delims The delimiters
 * @param[out] The subpart
 *
 * @retval TRUE There was a delimiter in the string
 * @retval FALSE The split hit the end of the string (the subpart contains the whole string)
 */
b8 str_view_split(str_view *str, const char *delims, str_view *out);

/**
 * @brief Trims a string view
 *
 * @param[in,out] view The string view
 * @param[in] type The type of trimming
 */
void str_view_trim(str_view *view, trim_type type);

/**
 * @brief Checks if two string views are equal (case sensitive)
 *
 * @param[in] a The first string view
 * @param[in] b The second string view
 *
 * @retval TRUE The views are equal
 * @retval FALSE The views are not equal
 */
b8 str_view_eq_view(str_view a, str_view b);

/**
 * @brief Checks if two string views are equal (case insensitive)
 *
 * @param[in] a The first string view
 * @param[in] b The second string view
 *
 * @retval TRUE The views are equal
 * @retval FALSE The views are not equal
 */
b8 str_view_eqi_view(str_view a, str_view b);

/**
 * @brief Checks if a string view is equal to a string (case sensitive)
 *
 * @param[in] a The first string view
 * @param[in] b The second string
 *
 * @retval TRUE The strings are equal
 * @retval FALSE The strings are not equal
 */
b8 str_view_eq(str_view a, const char *b);

/**
 * @brief Checks if a string view is equal to a string (case insensitive)
 *
 * @param[in] a The first string view
 * @param[in] b The second string
 *
 * @retval TRUE The strings are equal
 * @retval FALSE The strings are not equal
 */
b8 str_view_eqi(str_view a, const char *b);

/**
 * @brief Checks if a string is equal to a string (case sensitive)
 *
 * @param[in] a The first string
 * @param[in] b The second string
 *
 * @retval TRUE The strings are equal
 * @retval FALSE The strings are not equal
 */
b8 str_eq(const char *a, const char *b);

/**
 * @brief Checks if a string is equal to a string (case insensitive)
 *
 * @param[in] a The first string
 * @param[in] b The second string
 *
 * @retval TRUE The strings are equal
 * @retval FALSE The strings are not equal
 */
b8 str_eqi(const char *a, const char *b);

/**
 * @brief Copies a string view
 *
 * @param[in] view The string view
 *
 * @returns The copied string
 */
char *str_view_dup(str_view view);

/**
 * @brief Copies a string
 *
 * @param[in] str The string
 *
 * @returns The copied string
 */
char *str_dup(const char *str);

/**
 * @brief Checks if a string view starts with a string
 *
 * @param[in] a The string view
 * @param[in] prefix The prefix
 *
 * @retval TRUE The view starts with the prefix
 * @retval FALSE The view does not start with the prefix
 */
b8 str_view_starts_with_str(str_view a, const char *prefix);

/**
 * @brief Checks if a string view ends with a string
 *
 * @param[in] a The string view
 * @param[in] suffix The suffix
 *
 * @retval TRUE The view ends with the suffix
 * @retval FALSE The view does not end with the suffix
 */
b8 str_view_ends_with_str(str_view a, const char *suffix);

/**
 * @brief Checks if a string view starts with a set of characters
 *
 * @param[in] a The string view
 * @param[in] chars The characters
 *
 * @retval TRUE The view starts with one of the characters
 * @retval FALSE The view does not start with one of the characters
 */
b8 str_view_starts_with(str_view a, const char *chars);

/**
 * @brief Concatenates a string view to the destination string
 * If the destination points to a NULL pointer, it will be allocated
 *
 * @note The result of this function needs to be freed at some point
 *
 * @param[in,out] dest The destination string
 * @param[in] view The string view
 */
void str_cat_view_alloc(char **dest, str_view view);

/**
 * @brief Concatenates a string to the destination string
 *
 * @note The destination string must be large enough to hold the resulting string
 *
 * @param[in,out] dest The destination string
 * @param[in] view The string
 */
void str_cat_view(char *dest, str_view view);

/**
 * @brief Concatenates a string to the destination string
 * If the destination points to a NULL pointer, it will be allocated
 *
 * @note The result of this function needs to be freed at some point
 *
 * @param[in,out] dest The destination string
 * @param[in] str The string
 */
void str_cat_alloc(char **dest, const char *str);

/**
 * @brief Concatenates a string to the destination string
 *
 * @note The destination string must be large enough to hold the resulting string
 *
 * @param[in,out] dest The destination string
 * @param[in] str The string
 */
void str_cat(char *dest, const char *str);

/**
 * @brief Checks if a string contains a substring
 *
 * @param[in] haystack The string
 * @param[in] needle The substring
 *
 * @retval TRUE The string contains the substring
 * @retval FALSE The string does not contain the substring
 */
b8 str_contains_str(const char *haystack, const char *needle);

/**
 * @brief Checks if a string view contains a set of character
 *
 * @param[in] haystack The string
 * @param[in] needles The characters
 *
 * @retval TRUE The string contains one of the characters
 * @retval FALSE The string does not contain  one of the character
 */
b8 str_view_contains(str_view haystack, const char *needles);

/**
 * @brief Checks if a string view contains a character
 *
 * @param[in] haystack The string
 * @param[in] needle The character
 *
 * @retval TRUE The string contains the character
 * @retval FALSE The string does not contain the character
 */
b8 str_view_contains_char(str_view haystack, char needle);

/**
 * @brief Returns the length of a string
 *
 * @param[in] str The string
 *
 * @returns The length of the string
 */
u64 str_len(const char *str);
