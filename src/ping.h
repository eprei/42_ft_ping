#ifndef FT_PING_H
# define FT_PING_H

# include <stdbool.h>
# include <string.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/socket.h>
# include <time.h>
# include <netdb.h>
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

# define V_FLAG 1
# define TTL_FLAG 2
# define W_FLAG 3
# define w_FLAG 4
# define H_FLAG 5
# define PING_PKT_S 64              // ping packet size
# define PORT_NO 0                  // no port
# define SLEEP_RATE 1000000         // ping sleep rate (in microseconds)
# define TIMEOUT_DEFAULT 4          // timeout for receiving packets (in seconds)
# define DEFAULT_TTL 64
# define BUF_SIZE 128

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
# define FRAGMENT_REASSEMBLY_TIME_EXCEEDED "Fragment reassembly time exceeded"

// Custom Messages
# define MSG_UNKNOWN_HOST "ft_ping: unknown host"
# define MSG_HELP "Try 'ft_ping -?' or 'ft_ping -h' for more information."
# define MSG_ERROR_MISSING_HOST "ft_ping: missing host operand"
# define MSG_ERROR_UNRECOGNIZED_OPTION "ft_ping: unrecognized option"
# define MSG_ERROR_INVALID_OPTION "ft_ping: invalid option --"
# define MSG_INVALID_VALUE "ft_ping: invalid value"
# define USAGE_MSG "Usage: ft_ping [OPTION...] HOST ...           \n\
Send ICMP ECHO_REQUEST packets to network hosts.                  \n\
                                                                  \n\
Options:                                                          \n\
  <destination>              dns name or ip address               \n\
  -?                         print help and exit                  \n\
  -h                         print help and exit                  \n\
  -v                         verbose output                       \n\
  --ttl=N                    specify N as time-to-live            \n\
  -w <deadline>              reply wait <deadline> in seconds     \n\
  -W <timeout>               time to wait for response            "

struct ping_pkt {
    struct icmphdr icmp_header;
    char msg[PING_PKT_S - sizeof(struct icmphdr)];
};

typedef struct s_flags {
    bool v_flag;                  // Verbose output
    // bool ttl_flag;                // Time to live flag
    int ttl_value;                // Time to live value
    bool W_flag;                  // Wait W_num seconds for a response flag
    int W_num;                    // Wait W_num seconds for a response value
    bool w_flag;                  // Stop after w_num seconds flag
    int w_num;                    // Stop after w_num seconds value
} t_flags;

typedef struct s_rtt {
    long double rtt_value;
    struct s_rtt *next;
} t_rtt;

typedef struct s_config {
    t_flags flags;
    char *destination_host;
    char ip[INET_ADDRSTRLEN * sizeof(char)];
    int socket_fd;
    struct sockaddr_in sa;
    t_rtt *rtt_list;
    pid_t pid;
} t_config;

int ping(t_config *config);
bool parsing(int argc, char *argv[], t_config *ping);
void signal_handler(int signal);
bool convert_address(t_config *ping);
bool dns_lookup(t_config *ping);
void fill_packet(int *msg_count, struct ping_pkt *packet, pid_t pid);

void process_rtt(t_rtt **rtt_list, long double rtt);
long double get_last_rtt(const t_rtt *rtt_list);
long double get_min_rtt(t_rtt *rtt_list);
long double get_max_rtt(const t_rtt *rtt_list);
long double get_avg_rtt(const t_rtt *rtt_list);
long double get_mdev_rtt(t_rtt *rtt_list);
void free_rtt_list(t_rtt *rtt_node);

long double get_elapsed_time_ms(const struct timespec *time_start, struct timespec *time_end);
void get_timeout(const t_config *config, struct timeval *timeout);
bool is_time_limit_reached(const t_config *ping, struct timespec time_start);

void print_usage(void);
void print_error(char *error_msg);
void print_stats(const t_config *ping, int msgs_sent, int msgs_received, int errors);
void print_non_echo_icmp(const t_config *ping, const struct iphdr *base_ip_hdr, const struct icmphdr *icmp_hdr, char* sender_ip, const unsigned long received_packet_size);
void print_custom_message(const struct icmphdr* icmp_hdr);
void print_packet_content(const struct iphdr* base_ip_hdr);
void print_first_line(t_config* ping);
void print_option_need_argument(int flag_value);

#endif