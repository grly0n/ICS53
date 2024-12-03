#include <stdio.h>
#include <string.h>


int isWhitespace(char c) {
    return ((c == ' ') || (c == '\t') || (c == '\r') || (c == '\v') || (c == '\f') || (c == '\n'));
}

int main() {
    char* line = NULL;
    size_t size;

    int count = 0, lineCount = 0;
    int emptyFile = 1, lastLineChanged = 0;
    while ((getline(&line, &size, stdin)) != -1) {
        long int length = strlen(line);
        emptyFile = 0;
        lineCount++;
        
        int whitespaceOnly = 1, i, j;
        for (i = 0; i < length; i++) {
            if (!isWhitespace(line[i])) {
                whitespaceOnly = 0;
                break;
            }
        }

        int trailingWhitespace = 0;
        //printf("Last character %d\n", line[length-1]);
        for (j = length-1; j > i; j--) {
            if (j == length-1 && line[j] == '\n') continue;
            else if (isWhitespace(line[j])) { 
                //printf("Setting trailingWhitespace = 1 at index %d for %d\n", j, line[j]); 
                trailingWhitespace = 1;
            }
            if (!isWhitespace(line[j])) {
                //printf("Non-whitespace detected: line %d i %d j %d: %d\n", lineCount, i, j, line[j]); 
                break;
            }
        }

        //printf("trailingWhitespace = %d\n", trailingWhitespace);
        if (whitespaceOnly) {
            //printf("Line %d is whitespace only\n", lineCount);
            count++;
            lastLineChanged = 1;
        }
        else {
            int k = i;
            //printf("k: %d trailingWhitespace: %d\n", k, trailingWhitespace);
            if (k || trailingWhitespace) {
                //printf("incrementing count for line %d\n", lineCount); 
                count++;
                lastLineChanged = 1;
            } else lastLineChanged = 0;
            for (k; k <= j; k++) fprintf(stderr, "%c", line[k]);
            fprintf(stderr, "\n");
            //printf("line %d length: %ld\ti: %d\tj: %d\n", lineCount, length, i, j+1);
        }

    }
    if (emptyFile) printf("File was empty!\n");
    if (lastLineChanged) printf("(Last line was blank, only whitespace, or contained leading / trailing whitespace) %d\n", count);
    else printf("%d\n", count+1);
    return 0;
}
