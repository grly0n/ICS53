// Header file for hw1.c

#ifndef HW1_H
#define HW1_H

#include<stdio.h>
#include<stdlib.h>
#include"helpers1.h"

int parseArgs(char* argv[]);
int verifyArgs(int arg, int argc, char* argv[]);
int determineNumber(const char* arg);
int includeOutput(int argc, char* argv[]);

#endif
