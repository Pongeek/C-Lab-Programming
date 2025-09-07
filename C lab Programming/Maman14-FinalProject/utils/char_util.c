#include "../headers/char_util.h"

#include <string.h>

bool char_exists_in_string(char target, const char *str) {
    if (str == NULL) {
        return false;
    }
    return strchr(str, target) != NULL;
}

bool chars_are_equal(char a, char b){
    return (char)a == (char)b;
}

bool is_whitespace(char ch){
    return ch == ' ' || ch == '\t' || ch == '\r';
}

bool is_digit(char ch){
    return ch >= '0' && ch <= '9';
}

bool is_valid_identifier_start(char ch){
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

bool is_valid_identifier_char(char ch){
    return is_valid_identifier_start(ch) || is_digit(ch);
}