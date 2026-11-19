#include "ping.h"

static void fill_config(t_ping *config) {
    config->sa.sin_family = AF_INET;
    config->sa.sin_port = htons(PORT_NO);
}

static bool is_root(void) {
    if (geteuid() != 0) {
        fprintf(stderr, "ft_ping must be run as root\n");
        return false;
    }
    return true;
}

static bool create_raw_socket(t_ping *ping){
    ping->socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    if (ping->socket_fd < 0) {
        perror("socket");
        return false;
    }
    return true;
}

static bool verify_usage(const t_ping *ping) {
    if ( ping->destination_host == NULL) {
        print_error(MSG_ERROR_MISSING_HOST);
        print_error(MSG_HELP);
        return false;
    }
    return true;
}

int main(const int argc, char **argv) {

    signal(SIGINT, signal_handler);

    t_ping config = {0};
    fill_config(&config);

    if (!is_root() ||
        !parsing(argc, argv, &config) ||
        !verify_usage(&config) ||
        !create_raw_socket(&config)) {
        return 1;
    }

    return ping(&config);
}
