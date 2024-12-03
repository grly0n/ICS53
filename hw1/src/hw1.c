#include "hw1.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>


// determines which mode 53wc will enter into
int parseArgs(char* argv[]) {
    if (!strncmp(argv[1], "-L\0", 3)) return 1; 
    else if (!strncmp(argv[1], "-S\0", 3)) return 2;  
    else if (!strncmp(argv[1], "-N\0", 3)) return 3;
    else if (!strncmp(argv[1], "-C\0", 3)) return 4;
    else if (!strncmp(argv[1], "-W\0", 3)) return 5;
    else return 0;
}


// determines if the given argument structure is valid
int verifyArgs(int arg, int argc, char* argv[]) {
    switch(arg) {
        case 1:     // -L
            return (argc > 2);
        case 2:     // -S [-O]
        case 3:     // -N [-O]
            if (argc > 3) return 1;
            else if (argc == 3 && strncmp(argv[2], "-O\0", 3)) return 1;
            else return 0;
        case 4:     // -C NUM [-O]
            if (argc < 3 || argc > 4) return 1;
            else if (argc == 3 && !determineNumber(argv[2])) return 1;
            else return 0;
        case 5:     // -W [-O]
            if (argc > 3) return 1;
            else if (argc == 3 && strncmp(argv[2], "-O\0", 3)) return 1;
            else return 0;
    }
    return 1;
}


// given a char* determines if it consists of only digits 0-9
int determineNumber(const char* arg) {
    for (int i = 0; arg[i] != '\0'; i++) if (!isdigit(arg[i])) return 0;
    return 1;
}


// determines whether or not 53wc should print output
int includeOutput(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        if (!strncmp(argv[i], "-O\0", 3)) return 1;
    }
    return 0;
}


// main program
int main (int argc, char *argv[])
{
    // if no arguments are passed, exit with error code 1
    if (argc < 2) return 1;
    // if arguments are passed incorrectly, exit with error code 1
    int function = parseArgs(argv);
    if (verifyArgs(function, argc, argv)) return 1;
    // result will be -1 for all operations if an empty file is read from
    // otherwise, result will contain the count for the corresponding operation
    int result = 0;

    // switch statement for 5 possible valid arguments + default response
    switch (function) {
        case 1:     // -L
            result = countLines();
            break;
        case 2:     // -S (w/ or w/o -O)
            result = countSymbols(includeOutput(argc, argv));
            break;
        case 3:     // -N (w/ or w/o -O)
            result = countNumbers(includeOutput(argc, argv));
            break;
        case 4:     // -C
            result = countSpaces(atoi(argv[2]));
            break;
        case 5:     // -W
            result = countWhitespaces();
            break;
        default:
            return 1;
    }
    if (result < 0) return 2;
    else printf("%d\n", result);
	return 0;
}
