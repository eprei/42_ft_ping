#include "ft_ping.h"

bool parsing(const int argc, char **argv, t_ping *ping) {
    if (argc < 2) {
        fprintf(stderr, "%s\n", USAGE);
        return false;
    }

    for (int i = 1; i < argc; i++) {
        switch (is_flag(argv[i])) {
            case V_FLAG:
                ping->flags.v_flag = true;
                break;
            case TTL_FLAG:
                ping->flags.ttl_flag = true;
                if (!get_flag_value(argc, argv, ping, &i)) return false;
                break;
            case T_FLAG:
                ping->flags.t_flag = true;
                if (!get_flag_value(argc, argv, ping, &i)) return false;
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
                ping->flags.H_flag = true;
                break;
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
    if (strcmp("--ttl", str) == 0) {
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
        fprintf(stderr, "Error: %s requires a number between 0 and %d\n", argv[*i], INT_MAX);
        return false;
    }
    return true;
}