#include "ping.h"

void free_rtt_list(t_rtt *rtt_node) {
    while (rtt_node != NULL){
        t_rtt *to_free = rtt_node;
        rtt_node = rtt_node->next;
        free(to_free);
    }
}

void process_rtt(t_rtt **rtt_list, long double rtt){
    t_rtt *new_rtt_node = malloc(sizeof(t_rtt));

    if (new_rtt_node == NULL){
        exit_error("malloc error while adding new rtt_value to the list");
    }

    new_rtt_node->rtt_value = rtt;
    new_rtt_node->next = NULL;

    if (*rtt_list == NULL){
        *rtt_list = new_rtt_node;
    } else {
        t_rtt *iterator = *rtt_list;
        while (iterator->next != NULL ){
            iterator = iterator->next;
        }
        iterator->next = new_rtt_node;
    }
}

long double get_last_rtt(t_rtt *rtt_list){
    t_rtt *iterator = rtt_list;

    if (rtt_list == NULL){
        return 0.0;
    }

    while (iterator->next != NULL){
        iterator = iterator->next;
    }
    return iterator->rtt_value;
}

long double get_min_rtt(t_rtt *rtt_list){
    t_rtt *iterator = rtt_list;
    long double min_rtt = LDBL_MAX;

    if (rtt_list == NULL){
        return 0.0;
    }

    while (iterator != NULL){
        min_rtt = iterator->rtt_value < min_rtt ? iterator->rtt_value : min_rtt;
        iterator = iterator->next;
    }
    return min_rtt;
}

long double get_avg_rtt(t_rtt *rtt_list){
    t_rtt *iterator = rtt_list;
    int count = 0;
    long double sum = 0;

    if (rtt_list == NULL){
        return 0.0;
    }

    while (iterator != NULL){
        count++;
        sum += iterator->rtt_value;
        iterator = iterator->next;
    }
    return sum / count;
}

long double get_max_rtt(t_rtt *rtt_list){
    t_rtt *iterator = rtt_list;
    long double max_rtt = LDBL_MIN;

    if (rtt_list == NULL){
        return 0.0;
    }

    while (iterator != NULL){
        max_rtt = iterator->rtt_value > max_rtt ? iterator->rtt_value : max_rtt;
        iterator = iterator->next;
    }

    return max_rtt;
}

long double get_mdev_rtt(t_rtt *rtt_list){
    t_rtt *iterator = rtt_list;
    long double avg_rtt = get_avg_rtt(rtt_list);
    int count = 0;
    long double sum_sqr_dev_from_mean = 0;

    if (rtt_list == NULL){
        return 0.0;
    }

    while (iterator != NULL){
        sum_sqr_dev_from_mean += pow(iterator->rtt_value - avg_rtt, 2);
        count++;
        iterator = iterator->next;
    }

    long double average_of_sum_sqr_dev = sum_sqr_dev_from_mean / count;
    return sqrt(average_of_sum_sqr_dev);
}