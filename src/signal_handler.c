#include "ping.h"

extern volatile sig_atomic_t loop_running;

void signal_handler(int signal) {
    if (SIGINT == signal) {
        printf("\n");
        loop_running = 0;
    }
}
