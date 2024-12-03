// Define all helper functions for hw2 in this file
#include "helpers2.h"

void string_copy(char *dest, char *source) {
    if (!dest || !source) return;
    char *dest_ptr = dest, *source_ptr = source;
    while (*source_ptr != '\0') {
        *dest_ptr = *source_ptr;
        dest_ptr++;
        source_ptr++;
    }
    *dest_ptr = '\0';
}

void string_copy_lowercase(char *dest, char *source) {
    if (!dest || !source) return;
    char *dest_ptr = dest, *source_ptr = source;
    while (*source_ptr != '\0') {
        if (*source_ptr > 64 && *source_ptr < 91) *dest_ptr = *source_ptr+32;
        else *dest_ptr = *source_ptr;
        dest_ptr++;
        source_ptr++;
    }
    *dest_ptr = '\0';
}


int string_length(char *str) {
    if (!str) return 0;
    char *str_ptr = str;
    while (*str_ptr != '\0') str_ptr++;
    return str_ptr-str+1;
}


int containsSubstring(char *str, char *substr) {
    if (!str) return 0;
    char *str_ptr = str, *substr_ptr = substr;
    while (*str_ptr != '\0') {
        if (*str_ptr == *substr_ptr) {
            char *substr_ptr2 = substr_ptr;
            while (*str_ptr == *substr_ptr2) {
                str_ptr++;
                substr_ptr2++;
            }
            if (*substr_ptr2 == '\0') return 1;
        } else {
            str_ptr++;
        }
    }
    return 0;
}
