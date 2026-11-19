#include "ping.h"

static int is_flag(const char *str) {
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
    if (strcmp("-?", str) == 0 || strcmp("-h", str) == 0) {
        return H_FLAG;
    }
    if (str[0] == '-' && str[1] == '-' && strncmp("--ttl", str, 5) != 0) {
        fprintf(stderr, "%s '%s'\n", MSG_ERROR_UNRECOGNIZED_OPTION, str);
        print_error(MSG_HELP);
        return UNRECOGNIZED_FLAG;
    }
    if (str[0] == '-') {
        fprintf(stderr, "%s '%s'\n", MSG_ERROR_INVALID_OPTION, &str[1]);
        print_error(MSG_HELP);
        return UNRECOGNIZED_FLAG;
    }
    return NOT_A_FLAG;
}

static bool get_flag_value(const int *argc, char **argv, int *i, int *flag_value_store, const int flag) {
    if (argv[*i + 1] == NULL){
        print_option_need_argument(flag);
        exit(64);
    }
    if (*i + 1 < *argc && atol(argv[*i + 1]) < 1) {
        fprintf(stderr, "ft_ping: option value too small: %s\n", argv[*i+1]);
        return false;
    }
    if (*i + 1 < *argc && atol(argv[*i + 1]) > INT_MAX){
        fprintf(stderr, "ft_ping: option value too big: %s\n", argv[*i+1]);
        return false;
    }
    *flag_value_store = atoi(argv[++*i]);

    return true;
}

static bool get_ttl_value (t_ping *ping, const char *ttl_arg) {
    if (ttl_arg[6] == '\0') {
        fprintf(stderr, "ft_ping: option value too small: \n");
        return false;
    }
    if (strcmp(&ttl_arg[6], "0") == 0 ) {
        fprintf(stderr, "ft_ping: option value too small: %d\n", ping->flags.ttl_value);
        return false;
    }
    ping->flags.ttl_value = atoi(&ttl_arg[6]);
    if (ping->flags.ttl_value == 0) {
        fprintf(stderr, "%s (`%s' near `%s')\n", MSG_INVALID_VALUE, &ttl_arg[6], &ttl_arg[6]);
        return false;
    }
    if (ping->flags.ttl_value > 255) {
        fprintf(stderr, "ft_ping: option value too big: %d\n", ping->flags.ttl_value);
        return false;
    }
    if (ping->flags.ttl_value < 1) {
        fprintf(stderr, "ft_ping: option value too small: %d\n", ping->flags.ttl_value);
        return false;
    }

    return true;
}

bool parsing(const int argc, char **argv, t_ping *ping) {
    if (argc < 2) {
        print_error(MSG_ERROR_MISSING_HOST);
        print_error(MSG_HELP);
        exit(64);
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
                if (!get_flag_value(&argc, argv, &i, &ping->flags.W_num, W_FLAG)) return false;
                break;
            case w_FLAG:
                ping->flags.w_flag = true;
                if (!get_flag_value(&argc, argv, &i, &ping->flags.w_num, w_FLAG)) return false;
                break;
            case H_FLAG:
                print_usage();
                exit(0);
            case UNRECOGNIZED_FLAG:
                exit(64);
            default:
                if (ping->destination_host == NULL) {
                    ping->destination_host = argv[i];
                }
        }
    }
    return true;
}