#include "ft_ping.h"

int main(int argc, char **argv) {

    signal(SIGINT, signal_handler);

    t_ping ping = {0};
    init_struct(&ping, argv[0]);

    if (!is_root(&ping) || !parsing(argc, argv, &ping) || !create_raw_socket(&ping)) {
        return 1;
    }

    return ft_ping(&ping);
}

bool is_root(t_ping *ping) {
    if (getuid() != 0) {
        fprintf(stderr, "%s must be run as root\n", ping->binary_name);
        return false;
    }
    return true;
}

void init_struct(t_ping *ping, char *string) {
    ping->binary_name = string;
}

bool create_raw_socket(t_ping *ping){
    ping->socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    if (ping->socket_fd < 0) {
//        fprintf(stderr, "\nSocket file descriptor not received!\n");
        return false;
    } else {
//        printf("\nSocket file descriptor %d received\n", ping->socket_fd);
        return true;
    }
}