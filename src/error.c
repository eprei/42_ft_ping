#include "ping.h"

void exit_error(char *str){
    fprintf(stderr, "Error: %s\n", str);
    exit(1);
}
