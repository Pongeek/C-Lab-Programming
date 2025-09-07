#ifndef CHAR_UTIL
#define CHAR_UTIL

/*
 * The Char util:
 * this Char util provide helper functions for easy use of chars in c.
*/

#include <stdbool.h>
#include <ctype.h>


/**
 * Checks if a character is present in a given character pointer.
 * 
 * @param ch The character to search for.
 * @param string The character pointer to search in.
 * @return true if the character is found, false otherwise.
 * @note The string parameter must be null-terminated.
 */
bool char_exists_in_string(char target, const char *str);


/**
 * Checks if two charecters are equal.
 * 
 * @param ch1 The first character.
 * @param ch2 The second character.
 * @return true if the characters are equal, false otherwise.
 */
bool chars_are_equal(char a, char b);

/**
 * Checks if a character is whitespace.
 * 
 * @param ch The character to check.
 * @return true if the character is whitespace, false otherwise.
 * @note Whitespace characters include space, tab, newline, carriage return, vertical tab, and form feed.
 */
bool is_whitespace(char ch);

/**
 * Checks if a character is numeric (0-9).
 * 
 * @param ch The character to check.
 * @return true if the character is a digit, false otherwise.
 */
bool is_digit(char ch);

/**
 * Checks if a character is a valid identifier starter character.
 * 
 * @param ch The character to check.
 * @return true if the character is a valid identifier starter, false otherwise.
 * @note Identifier starter characters include letters (a-z, A-Z) and underscores (_).
 */
bool is_valid_identifier_start(char ch);

/**
 * Checks if a character is a valid identifier character.
 * 
 * @param ch The character to check.
 * @return true if the character is a valid identifier character, false otherwise.
 * @note Identifier characters include letters (a-z, A-Z), digits (0-9), and underscores (_).
 */
bool is_valid_identifier_char(char ch);

#endif /* CHAR_UTIL */