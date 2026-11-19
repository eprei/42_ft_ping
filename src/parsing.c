#include "ping.h"

static int is_flag(const char *str) {
    if (strcmp("-v", str) == 0) {
        return IS_V_FLAG;
    }
    if (strncmp("--ttl", str, 5) == 0) {
        return IS_TTL_FLAG;
    }
    if (strcmp("-W", str) == 0) {
        return IS_W_FLAG;
    }
    if (strcmp("-w", str) == 0) {
        return IS_w_FLAG;
    }
    if (strcmp("-?", str) == 0 || strcmp("-h", str) == 0) {
        return IS_H_FLAG;
    }
    if (str[0] == '-' && str[1] == '-') {
        fprintf(stderr, "%s '%s'\n", MSG_ERROR_UNRECOGNIZED_OPTION, str);
        print_error(MSG_HELP);
        exit(64);
    }
    if (str[0] == '-') {
        fprintf(stderr, "%s '%c'\n", MSG_ERROR_INVALID_OPTION, str[1]);
        print_error(MSG_HELP);
        exit(64);
    }
    return false;
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

static bool get_ttl_value (t_config *ping, const char *ttl_arg) {
    if (strcmp(ttl_arg, "--ttl") == 0) {
        fprintf(stderr, "ft_ping: option '--ttl' requires an argument\n");
        fprintf(stderr, "%s\n", MSG_HELP);
        exit(64);
    }
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

bool parsing(const int argc, char **argv, t_config *config) {
    if (argc < 2) {
        print_error(MSG_ERROR_MISSING_HOST);
        print_error(MSG_HELP);
        exit(64);
    }

    for (int i = 1; i < argc; i++) {
        switch (is_flag(argv[i])) {
            case IS_V_FLAG:
                config->flags.v_flag = true;
                break;
            case IS_TTL_FLAG:
                if (!get_ttl_value(config, argv[i])) return false;
                break;
            case IS_W_FLAG:
                config->flags.W_flag = true;
                if (!get_flag_value(&argc, argv, &i, &config->flags.W_num, IS_W_FLAG)) return false;
                break;
            case IS_w_FLAG:
                config->flags.w_flag = true;
                if (!get_flag_value(&argc, argv, &i, &config->flags.w_num, IS_w_FLAG)) return false;
                break;
            case IS_H_FLAG:
                print_usage();
                exit(0);
            default:
                if (config->destination_host == NULL) {
                    config->destination_host = argv[i];
                }
        }
    }
    return true;
}