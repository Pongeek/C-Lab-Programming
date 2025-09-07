#include <string.h>
#include <stdbool.h>
#include "../headers/safe_allocations.h"
#include "../headers/string_util.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

static unsigned next_power_of_two(unsigned x);

String string_create() {
    String str;
    str.capacity = 8; /* Initial capacity */
    str.length = 0; /* Empty string */
    str.data = (char *) safe_calloc(8, sizeof(char));
    return str;
}

void string_free(String str) {
    if (str.data == NULL) {
        printf("Warning: Attempting to free a NULL pointer.\n");
        return;
    }
    free(str.data);
    str.data = NULL; /* Set to NULL after freeing */
}

String string_create_from_cstr(const char *data) {
    String str;
    size_t len;

    len = strlen(data);
    str.capacity = MAX(next_power_of_two(len + 1), 8);
    str.length = len;
    str.data = (char *) safe_calloc(str.capacity, sizeof(char));

    strcpy(str.data, data);
    return str;
}

void string_append_char(String *str, char ch) {
    if (ch == '\0') return;

    if (str->length + 1 >= str->capacity) {
        str->capacity *= 2;
        str->data = safe_realloc(str->data, str->capacity);
    }

    str->data[str->length++] = ch;
    str->data[str->length] = '\0';
}

void string_debug_info(String str) {
    printf("String capacity: %u\n", str.capacity);
    printf("String length: %u\n", str.length);
    printf("String content: %s\n", str.data);
}

void string_append_cstr(String *str, const char *cstr) {
    size_t len;

    len = strlen(cstr);

    while (str->length + len + 1 > str->capacity) {
        str->capacity *= 2;
        str->data = safe_realloc(str->data, str->capacity);
    }

    strcpy(str->data + str->length, cstr);
    str->length += len;
}

void string_append(String *dest, String src) {
    string_append_cstr(dest, src.data);
}

char string_char_at(String str, unsigned int index) {
    return (index < str.length) ? str.data[index] : '\0';
}

bool string_equals(String str1, String str2) {
    if (str1.data == NULL || str2.data == NULL) {
        return str1.data == str2.data;
    }
    return (str1.length == str2.length) && (strcmp(str1.data, str2.data) == 0);
}

bool string_equals_cstr(String str1, const char *str2) {
    return (str1.length == strlen(str2)) && (strcmp(str1.data, str2) == 0);
}

int string_length(String str) {
    return str.length; /* Length of the string. */
}

String string_substring(String str, int start, int end) {
    int i;
    String result;

    result = string_create();
    for (i = start; i <= end && i < str.length; i++) {
        string_append_char(&result, str.data[i]);
    }
    return result;
}

void string_remove_range(String *str, int start, int end) {
    int shift;

    if (start < 0 || end >= str->length || start > end) return;

    shift = end - start + 1;
    memmove(str->data + start, str->data + end + 1, str->length - end);
    str->length -= shift;
    str->data[str->length] = '\0';
}

void string_replace(String *str, String pattern, String replacement) {
    String result;
    int i;

    result = string_create();
    i = 0;

    while (i < str->length) {
        if (strncmp(str->data + i, pattern.data, pattern.length) == 0) {
            string_append(&result, replacement);
            i += pattern.length;
        } else {
            string_append_char(&result, str->data[i]);
            i++;
        }
    }

    string_free(*str);
    *str = result;
}

static unsigned next_power_of_two(unsigned x) {
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;
}

