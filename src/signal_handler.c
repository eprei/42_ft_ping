#include "ping.h"

extern volatile sig_atomic_t loop_running;

void signal_handler(const int signal) {
    if (SIGINT == signal) {
        loop_running = 0;
    }
}
