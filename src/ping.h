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
# define PORT_NO 0                  // no port
# define SLEEP_RATE 1000000         // ping sleep rate (in microseconds)
# define RECV_TIMEOUT 4             // timeout for receiving packets (in seconds)
# define DEFAULT_TTL 64
# define RECV_BUFFER_SIZE 128

# define UNKNOWN_HOST_MSG "%s: %s: Name or service not known\n"

// ICMP messages
# define HOST_UNREACHABLE "Destination Host Unreachable"
# define NET_UNREACHABLE "Destination Net Unreachable"
# define TTL_EXCEEDED "Time to live exceeded"
# define PROTOCOL_UNREACHABLE "Protocol Unreachable"
# define PORT_UNREACHABLE "Port Unreachable"
# define FRAGMENTATION_NEEDED "Fragmentation needed and DF set"
# define SOURCE_ROUTE_FAILED "Source route failed"
# define NETWORK_UNKNOWN "Destination network unknown"
# define HOST_UNKNOWN "Destination host unknown"
# define HOST_ISOLATED "Source host isolated"
# define NETWORK_PROHIBITED "Communication with destination network is administratively prohibited"
# define HOST_PROHIBITED "Communication with destination host is administratively prohibited"
# define NETWORK_UNREACHABLE_TOS "Destination network unreachable for type of service"
# define HOST_UNREACHABLE_TOS "Destination host unreachable for type of service"
# define PACKET_FILTERED "Communication administratively prohibited"
# define PRECEDENCE_VIOLATION "Host precedence violation"
# define PRECEDENCE_CUTOFF "Precedence cutoff in effect"
# define SOURCE_QUENCH "Source quench"
# define REDIRECT_NETWORK "Redirect datagram for the network"
# define REDIRECT_HOST "Redirect datagram for the host"
# define REDIRECT_NETWORK_TOS "Redirect datagram for the type of service and network"
# define REDIRECT_HOST_TOS "Redirect datagram for the type of service and host"
# define FRAGMENT_REASSEMBLY_TIME_EXCEEDED "Fragment reassembly time exceeded"
# define PARAMETER_PROBLEM_POINTER "Pointer indicates the error"
# define PARAMETER_PROBLEM_OPTION_MISSING "Missing a required option"
# define PARAMETER_PROBLEM_BAD_LENGTH "Bad length"
# define TIMESTAMP_REQUEST "Timestamp request"
# define TIMESTAMP_REPLY "Timestamp reply"
# define INFO_REQUEST "Information request"
# define INFO_REPLY "Information reply"
# define ADDRESS_MASK_REQUEST "Address mask request"
# define ADDRESS_MASK_REPLY "Address mask reply"

# define USAGE_MSG "Usage                                         \n\
  ft_ping [options] <destination>                                 \n\
                                                                  \n\
Options:                                                          \n\
  <destination>              dns name or ip address               \n\
  -h                         print help and exit                  \n\
  -t <time to live>          define time to live                  \n\
  -v                         verbose output                       \n\
  -w <deadline>              reply wait <deadline> in seconds     \n\
  -W <timeout>               time to wait for response            "

# define USAGE_ERROR "ft_ping: usage error: Destination address required"

struct ping_pkt {
    struct icmphdr icmp_header;
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
    pid_t ping_id;
} t_ping;


bool parsing(int argc, char *argv[], t_ping *ping);
int ft_ping(t_ping *ping);
void signal_handler(int signal);
bool convert_address(t_ping *ping);
void init_struct(t_ping *ping, char *string);
bool is_root(const t_ping *ping);
bool dns_lookup(t_ping *ping);
bool create_raw_socket(t_ping *ping);
void send_ping(t_ping *pin);
unsigned short checksum(void *b, int len);
int is_flag(const char *str);
bool get_flag_value(const int *argc, char **argv, int *i, int *flag_value_store, int flag);
bool get_ttl_value(t_ping *ping, char *arg);
void print_usage();
void print_usage_error();
bool verify_usage(const t_ping *ping);
void fill_packet(int *msg_count, struct ping_pkt *packet);
void exit_error(char *str);
void process_rtt(t_rtt **rtt_list, long double rtt);
long double get_last_rtt(t_rtt *rtt_list);
long double get_min_rtt(t_rtt *rtt_list);
long double get_max_rtt(t_rtt *rtt_list);
long double get_avg_rtt(t_rtt *rtt_list);
long double get_mdev_rtt(t_rtt *rtt_list);
void free_rtt_list(t_rtt *rtt_node);
void print_stats(const t_ping *ping, int msg_count, int msg_received_count, int errors);
long double get_elapsed_time_ms(const struct timespec *time_start, struct timespec *time_end);
void get_timeout(const t_ping *ping, struct timeval *timeout);
bool time_limit_reached(const t_ping *ping, struct timespec time_start);

#endif
