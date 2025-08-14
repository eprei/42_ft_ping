#include "ft_ping.h"

bool parsing(const int argc, char **argv, t_ping *ping) {
    if (argc < 2) {
        print_usage_error();
        return false;
    }

    for (int i = 1; i < argc; i++) {
        switch (is_flag(argv[i])) {
            case V_FLAG:
                ping->flags.v_flag = true;
                break;
            case TTL_FLAG:
                ping->flags.ttl_flag = true;
                if (!get_ttl_value(ping, argv[i])) return false;
                break;
            case W_FLAG:
                ping->flags.W_flag = true;
                if (!get_flag_value(argc, argv, ping, &i)) return false;
                break;
            case w_FLAG:
                ping->flags.w_flag = true;
                if (!get_flag_value(argc, argv, ping, &i)) return false;
                break;
            case H_FLAG:
                print_usage();
                return false;
            default:
                ping->destination_host = argv[i];
        }
    }
    return true;
}

int is_flag(const char *str) {
    if (strcmp("-v", str) == 0) {
        return V_FLAG;
    }
    if (strncmp("--ttl=", str, 6) == 0) {
        return TTL_FLAG;
    }
    if (strcmp("-W", str) == 0) {
        return W_FLAG;
    }
    if (strcmp("-w", str) == 0) {
        return w_FLAG;
    }
    if (strcmp("-h", str) == 0){
        return H_FLAG;
    }
    return NOT_A_FLAG;
}

bool get_flag_value(int argc, char **argv, t_ping *ping, int *i) {
    if ((*i + 1 < argc && atoi(argv[*i + 1]) > 0) || *argv[*i + 1] == '0') {
        ping->flags.w_num = atoi(argv[++*i]);
    } else {
        fprintf(stderr, "ft_ping: invalid argument: '%s': Numerical result out of range\n", argv[*i+1]);
        return false;
    }
    return true;
}

bool get_ttl_value (t_ping *ping, char *arg) {
    ping->flags.ttl_value = atoi(&arg[6]);

    if (1 <= ping->flags.ttl_value && ping->flags.ttl_value <= 255){
        return true;
    }

    fprintf(stderr, "ft_ping: invalid argument: '%d': out of range: 1 <= value <= 255\n", ping->flags.ttl_value);
    return false;
}

void print_usage() {
    fprintf(stderr, "%s\n", USAGE_MSG);
}
