#include "ping.h"

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
            case T_FLAG:
                ping->flags.t_flag = true;
                if (!get_ttl_value(&argc, argv, &i, ping)) return false;
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
    if (strcmp("-t", str) == 0) {
        return T_FLAG;
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

void print_option_need_argument(const int flag_value)
{
    char option;

    switch (flag_value){
        case T_FLAG:
            option = 't';
            break;
        case W_FLAG:
            option = 'W';
            break;
        case w_FLAG:
            option = 'w';
            break;
        default:
            option = ' ';
            break;
    }
    fprintf(stderr, "ft_ping: option requires an argument -- '%c'\n\n", option);
    fprintf(stderr, "%s\n", USAGE_MSG);
}

bool get_flag_value(const int *argc, char **argv, int *i, int *flag_value_store, const int flag) {
    if (argv[*i + 1] == NULL){
        print_option_need_argument(flag);
        return false;
    }
    if ((*i + 1 < *argc && atoi(argv[*i + 1]) > 0) || *argv[*i + 1] == '0') {
        *flag_value_store = atoi(argv[++*i]);
    } else {
        fprintf(stderr, "ft_ping: invalid argument: '%s': Numerical result out of range\n", argv[*i+1]);
        return false;
    }
    return true;
}

bool get_ttl_value (const int *argc, char **argv, int *i, t_ping *ping) {
    if (argv[*i + 1] == NULL){
        print_option_need_argument(T_FLAG);
        return false;
    }

    if (!(*i + 1 < *argc && atoi(argv[*i + 1]) != 0) || *argv[*i + 1] == '0'){
            fprintf(stderr, "ft_ping: invalid argument: '%s'\n", argv[*i + 1]);
        return false;
    }

    ping->flags.t_value = atoi(argv[++*i]);

    if (1 <= ping->flags.t_value && ping->flags.t_value <= 255){
        return true;
    }

    fprintf(stderr, "ft_ping: invalid argument: '%d': out of range: 1 <= value <= 255\n", ping->flags.t_value);
    return false;
}

void print_usage() {
    fprintf(stderr, "%s\n", USAGE_MSG);
}
