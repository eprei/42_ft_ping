#include "ping.h"

bool is_time_limit_reached(const t_ping *ping, const struct timespec time_start) {
    struct timespec time_end;

    if (ping->flags.w_flag){
        return get_elapsed_time_ms(&time_start, &time_end) / 1000 > ping->flags.w_num;
    }
    return false;
}

long double get_elapsed_time_ms(const struct timespec *time_start, struct timespec *time_end) {
    clock_gettime(CLOCK_MONOTONIC, time_end);
    const double nanoseconds_elapsed = ((double)(time_end->tv_nsec - (*time_start).tv_nsec)) / 1000000.0;
    const long double total_elapsed_time_ms = (time_end->tv_sec - time_start->tv_sec) * 1000.0 + nanoseconds_elapsed;
    return total_elapsed_time_ms;
}

void get_timeout(const t_ping *ping, struct timeval *timeout) {
    timeout->tv_sec = ping->flags.W_flag == true ? ping->flags.W_num: RECV_TIMEOUT;
    timeout->tv_usec = 0;
}