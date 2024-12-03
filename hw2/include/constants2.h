// DO NOT MODIFY THIS FILE
// This constants can be replaced with alternates during grading

#ifndef CONSTANTS_2_H
#define CONSTANTS_2_H

#define USAGE_MSG "53music [-n NUM] [-o OUTFILE] [-i INFILE] [-l] [-v] -D DATE | -G GENRE | -K KEYWORD\n"                                     \
                  "\n  -H          Prints the usage statement to STDOUT. All other arguments are ignored."\
                  "\n  -D DATE     Search for songs last played on or after DATE (format mm/dd/yyyyy)"\
                  "\n  -G GENRE    Search by songs in a GENRE"\
                  "\n  -K KEYWORD  Search by KEYWORD in artist, title, or album fields"\
                  "\n  -n NUM      Print top NUM results of the search."\
                  "\n  -l          Print only liked songs. With -n NUM, returns NUM liked songs"\
                  "\n  -o          Output is written to OUTFILE if specified. If unspecified, output is written to stdout"\
                  "\n  -i          Input is taken from INFILE if specified. If unspecified, input is read from stdin\n"
#endif

// MACRO for color highlighting & BOLD for verbose printing (Black foreground and Green Background)
#define COLOR_START "\e[1;30;47m"
#define COLOR_RESET "\e[0m"
