#ifndef STRING_UTIL
#define STRING_UTIL

/*
 * The String util:
 * this string util provide the String struct and its helper functions for easy use of strings in c.
 * 
 * How the String struct works:
 * each string is allocated with memory and every time that we want to add stuff to it we just create a new buffer with double the capacity.
 * in the start we will allocate a buffer of 8 chars (including the \0 char) and if we see that the length that points to the \0 char in the
 * struct is in the end of the buffer we would allocate the buffer again with more memory.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct String {
    unsigned int capacity;  /* the capacity of the buffer allocated to storing the string data (including the \0 char) */
    unsigned int length; /* the length of the String that has the end of the string, the \0 char (can be use to itself as the length of the string)*/
    char * data; /* a pointer to the data */
} String;

/**
 * Initialize a new empty String with default capacity.
 *
 * @return A new String with initial capacity of 8 bytes.
 */
String string_create(void);

/**
 * Free the memory allocated for a String.
 *
 * @param str The String to be deallocated.
 */
void string_free(String str);

/**
 * Create a new String initialized with given data.
 *
 * @param data Null-terminated string to initialize with.
 * @return A new String containing the given data.
 */
String string_create_from_cstr(const char *data);

/**
 * Print debug information about a String.
 *
 * @param str The String to display information about.
 */
void string_debug_info(String str);

/**
 * Append a single character to the end of a String.
 *
 * @param str Pointer to the String to modify.
 * @param ch Character to append.
 */
void string_append_char(String * str, char ch);

/**
 * Append a C-style string to the end of a String.
 *
 * @param str Pointer to the String to modify.
 * @param cstr Null-terminated string to append.
 */
void string_append_cstr(String *str, const char *cstr);

/**
 * Append one String to another.
 *
 * @param dest Pointer to the destination String.
 * @param src The source String to append.
 */
void string_append(String *dest, String src);

/**
 * Get a character from a String at a specific fileIndex.
 *
 * @param str The String to access.
 * @param index The index of the character to retrieve.
 * @return The character at the given index, or '\0' if out of bounds.
 */
char string_char_at(String str, unsigned int index);

/**
 * Compare two Strings for equality.
 *
 * @param str1 The first String to compare.
 * @param str2 The second String to compare.
 * @return true if the Strings are equal, false otherwise.
 */
bool string_equals(String str1, String str2);

/**
 * Compare a String with a C-style string for equality.
 *
 * @param str The String to compare.
 * @param cstr The null-terminated C-style string to compare.
 * @return true if the strings are equal, false otherwise.
 */
bool string_equals_cstr(String str1, const char *str2);

/**
 * Get the length of a String.
 *
 * @param str The String to measure.
 * @return The number of characters in the String.
 */
int string_length(String str);

/**
 * Extract a substring from a String.
 *
 * @param str The source String.
 * @param start The starting index of the substring.
 * @param end The ending index of the substring (inclusive).
 * @return A new String containing the extracted substring.
 */
String string_substring(String str, int start, int end);

/**
 * Remove a portion of a String.
 *
 * @param str Pointer to the String to modify.
 * @param start The starting index of the portion to remove.
 * @param end The ending index of the portion to remove (inclusive).
 */
void string_remove_range(String *str, int start, int end);

/**
 * Replace all occurrences of a pattern in a String.
 *
 * @param str Pointer to the String to modify.
 * @param pattern The pattern String to search for.
 * @param replacement The String to replace the pattern with.
 */
void string_replace(String *str, String pattern, String replacement);

#endif /* STRING_UTIL */