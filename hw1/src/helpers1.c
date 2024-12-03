// Define all helper functions for hw1 in this file
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "helpers1.h"


// counts the number of newlines from stdin
int countLines() {
    char c = getchar();
    if (c == EOF) return -1;
    int lineCount = 0;

    while (c != EOF) {
        if (c == '\n') lineCount++;
        else if (!isascii(c)) return lineCount;
        c = getchar();
    }
    return lineCount;
}


// given a character, determine the symbol-only index (for -S -O)
int determineIndex(int c) {
    if (c < '0') return c-'!';
    else if (c < 'A') return c-'!'-10;
    else if (c < 'a') return c-'!'-36;
    else return c-'!'-62;
}


// given a symbol-only index, determine the character (for -S -O)
int determineChar(int i) {
    if (i < 15) return i+'!';
    else if (i < 22) return i+'!'+10;
    else if (i < 28) return i+'!'+36;
    else return i+'!'+62;
}


// counts the number of symbols (standard visible non-alphanumeric characters) from stdin
int countSymbols(int output) {
    char c = getchar();
    if (c == EOF) return -1;
    int symbolCount = 0;
    int frequency[32]  = {0};

    while (c != EOF) {
        if (!isascii(c)) {
            goto returnOutput;
        }
        else if ((32 < c && c < 127) && !(c >= 48 && c <= 57) && !(c >= 65 && c <= 90) && !(c >= 97 && c <= 122)) {    
            symbolCount++;
            int index = determineIndex(c);
            frequency[index]++;
        }
        c = getchar();
    }
    returnOutput:
    if (output) {
        for (int i = 0; i < 32; i++) {
            if (frequency[i] != 0) fprintf(stderr, "%c %d\n", determineChar(i), frequency[i]);
        }
    }
    return symbolCount;
}


// counts the number of digit sequences from stdin
int countNumbers(int output) {
    char c = getchar();
    if (c == EOF) return -1;
    int numberCount = 0;

    while (c != EOF) {
        if (!isascii(c)) return numberCount;
        else if (48 <= c && c <= 57) {
            while (48 <= c && c <= 57) {
                if (output) fprintf(stderr, "%c", c);
                c = getchar();
            }
            numberCount++;
            if (output) fprintf(stderr, "\n");
        }
        c = getchar();
    }
    return numberCount;
}


// compresses all instances of spacesCount consecutive spaces into a tab character
int countSpaces(int spacesCount) {
    char* line = NULL;
    size_t size;
    int resultCount = 0, fileEmpty = 1;

    while ((getline(&line, &size, stdin)) != -1) {
        long int length = strlen(line);
        fileEmpty = 0;
        for (int i = 0; i < length; i++) {
            if (!isascii(line[i])) {
                free(line);
                return resultCount;
            }
            if (line[i] == ' ') {
                int count_2 = 0, j = i;
                char buffer[spacesCount];
                char* buffer_ptr = buffer;
                for (j; line[j] == ' ' && j < length && count_2 < spacesCount; j++) {
                    count_2++;
                    *buffer_ptr = line[j]; buffer_ptr++;}
                if (count_2 == spacesCount) {putc('\t', stderr); resultCount++;}
                else {*buffer_ptr = '\0'; fputs(buffer, stderr);}
                i=j-1;
            } else putc(line[i], stderr);
        }
    }
    free(line);
    if (fileEmpty) return -1; else return resultCount;
}


int isWhiteSpace(char c) {
    return ((c == ' ') || (c == '\t') || (c == '\r') || (c == '\v') || (c == '\f') || (c == '\n'));
}

// counts the number of blank lines, whitespace only lines, and lines that have trailing or leading whitespace
int countWhitespaces() {
    char* line = NULL;
    size_t size;
    int whiteSpaceCount = 0, fileEmpty = 1;
    
    while ((getline(&line, &size, stdin)) != -1) {
        long int length = strlen(line);
        fileEmpty = 0;
        
        int whiteSpaceOnly = 1, i, j;
        // iterate forward through the line until a non-whitespace character is found
        for (i = 0; i < length; i++) {
            if (!isWhiteSpace(line[i])) {
                whiteSpaceOnly = 0;
                break;
            }
        }

        int trailingWhiteSpace = 0;
        // iterate backward through the line until a non-whitespace character is found
        // note: starts at length-2 to account for the \n at the end of every line, which is
        // not considered trailing whitespace
        for (j = length-2; j > i; j--) {
            if (isWhiteSpace(line[j])) trailingWhiteSpace = 1;
            else break;
        }
        
        int k = i, k_2 = i;
        char buffer[j-i+2];
        char* buffer_ptr = buffer;
        for (k; k < j+1; k++) {
            if (!isascii(line[k])) {goto returnOutput;}
            *buffer_ptr = line[k];
            buffer_ptr++;
        }
        *buffer_ptr = '\n';
        buffer_ptr++;
        *buffer_ptr = '\0';


        // check if the line only contains whitespace
        if (whiteSpaceOnly) {
            whiteSpaceCount++;
        } else {
            // check if the line contains leading or trailing whitespace
            // the starting index will be non-zero if there is leading whitespace
            if (k_2 || trailingWhiteSpace) {
                whiteSpaceCount++;
            }
            fprintf(stderr, "%s", buffer);
       }
    }
    returnOutput:
    free(line);
    if (fileEmpty) return -1;
    // if the last line was already counted, return the current count to avoid double counting the "\n EOF" case
    // otherwise, add 1 to the count to account for "\n EOF" being considered a blank line
    else return whiteSpaceCount+1;

}
