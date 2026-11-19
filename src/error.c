#include "ping.h"

void exit_error(char *str, const int ret_value){
    fprintf(stderr, "Error: %s\n", str);
    exit(ret_value);
}
