#include "hw0.h"
#include "helpers0.h"

int main (int argc, char *argv[])
{

	//Comment/Delete this print statement and insert your hw0 code here 
	//printf("Hello ICS53 student!\n"); 
    int num_chars = 0;
    for (int i = 0; i < argc; i++) {
        num_chars += printArg(argv[i], i);
    }
    printf("Total characters: %d\n", num_chars);

	return 0;
}
