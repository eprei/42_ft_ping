#ifndef FT_PING_H
# define FT_PING_H

# include <stdbool.h>
# include <string.h>
# include <stdio.h>
# include <stdlib.h>
# include <ctype.h>
# include <sys/socket.h>
# include <time.h>
# include <netdb.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netinet/ip_icmp.h>
# include <arpa/inet.h>
# include <signal.h>
# include <unistd.h>
# include <sys/types.h>
# include <limits.h>
# include <float.h>
# include <math.h>
# include <errno.h>

# define NOT_A_FLAG 0
# define V_FLAG 1
# define TTL_FLAG 2
# define W_FLAG 3
# define w_FLAG 4
# define H_FLAG 5

# define PING_PKT_S 64              // ping packet size
# define PORT_NO 0                  // automatic port number TODO why ?
# define SLEEP_RATE 1000000         // ping sleep rate (in microseconds)
# define RECV_TIMEOUT 4             // timeout for receiving packets (in seconds)
# define DEFAULT_TTL 60

# define UNKNOWN_HOST_MSG "%s: %s: Name or service not known\n"
# define HOST_UNREACHABLE "Destination Host Unreachable"

# define USAGE_MSG "Usage                                       \n\
ft_ping [options] <destination>                                 \n\
                                                                \n\
<destination>              dns name or ip address               \n\
                                                                \n\
        Options:                                                \n\
-h                         print help and exit                  \n\
--ttl <time to live>       define time to live                  \n\
-v                         verbose output                       \n\
-w <deadline>              reply wait <deadline> in seconds     \n\
-W <timeout>               time to wait for response            "

# define USAGE_ERROR "ft_ping: usage error: Destination address required"

struct ping_pkt {
    struct icmphdr icmp_header; // TODO refactor rename for something more meaningful
    char msg[PING_PKT_S - sizeof(struct icmphdr)];
};

typedef struct s_flags {
    bool v_flag;        // Verbose output
    bool ttl_flag;      // Time to live flag
    int ttl_value;      // Time to live value
    bool W_flag;        // Wait W_num seconds for a response flag
    int W_num;          // Wait W_num seconds for a response
    bool w_flag;        // Stop after w_num seconds flag
    int w_num;          // Stop after w_num seconds
    bool H_flag;        // Help flag
} t_flags;

typedef struct s_rtt {
    long double rtt_value;
    struct s_rtt *next;
} t_rtt;

typedef struct s_ping {
    char *binary_name;
    t_flags flags;
    char *destination_host;
    char ip[INET_ADDRSTRLEN * sizeof(char)];
    int socket_fd;
    struct sockaddr_in sa;
    t_rtt *rtt_list;
} t_ping;


bool parsing(int argc, char *argv[], t_ping *ping);
int ft_ping(t_ping *ping);
void signal_handler(int signal);
bool convert_address(t_ping *ping);
void init_struct(t_ping *ping, char *string);
bool is_root(t_ping *ping);
bool dns_lookup(t_ping *ping);
bool create_raw_socket(t_ping *ping);
void send_ping(t_ping *pin);
unsigned short checksum(void *b, int len);
int is_flag(const char *str);
bool get_flag_value(int argc, char **argv, int *i, int *flag_value);
bool get_ttl_value(t_ping *ping, char *arg);
void print_usage();
void print_usage_error();
bool verify_usage(t_ping *ping);
void fill_packet(int *msg_count, struct ping_pkt *pckt);
void exit_error(char *str);
void process_rtt(t_rtt **rtt_list, long double rtt);
long double get_last_rtt(t_rtt *rtt_list);
long double get_min_rtt(t_rtt *rtt_list);
long double get_max_rtt(t_rtt *rtt_list);
long double get_avg_rtt(t_rtt *rtt_list);
long double get_mdev_rtt(t_rtt *rtt_list);
void free_rtt_list(t_rtt *rtt_node);
void print_statistics(const t_ping *ping, int msg_count, int msg_received_count, long double total_time, int errors);
long double elapsed_time_ms(struct timespec *time_start, struct timespec *time_end);
void get_timeout(t_ping *ping, struct timeval *timeout);
bool time_limit_reached(t_ping *ping, struct timespec time_start);

#endif
