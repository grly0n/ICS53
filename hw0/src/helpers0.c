#include "helpers0.h"
#include <string.h>

//Function to print out a single arugment to the screen
int printArg(char * arg_str, int pos){

	//Insert your code here
    printf("argv[%d]: %s\n", pos, arg_str);
	if (pos > 0) return strlen(arg_str);
    else return 0;
}
