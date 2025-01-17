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

# define NOT_A_FLAG 0
# define V_FLAG 1
# define TTL_FLAG 2
# define T_FLAG 3
# define W_FLAG 4
# define w_FLAG 5
# define H_FLAG 6

# define PING_PKT_S 64               // ping packet size
# define PORT_NO 0                   // automatic port number
# define PING_SLEEP_RATE 1000000     // ping sleep rate (in microseconds)
# define RECV_TIMEOUT 1              // timeout for receiving packets (in seconds)

# define UNKNOWN_HOST "%s: %s: Name or service not known"

# define USAGE "Usage                                           \n\
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

// Ping packet structure
struct ping_pkt {
    struct icmphdr hdr;
    char msg[PING_PKT_S - sizeof(struct icmphdr)];
};

typedef struct s_flags {
    bool v_flag;   // Verbose output
    bool ttl_flag; // Time to live flag
    int ttl_num;   // Time to live number
    bool t_flag;   // Type-of-service flag
    int t_num;     // Type-of-service number
    bool W_flag;   // Wait W_num seconds for a response flag
    int W_num;     // Wait W_num seconds for a response
    bool w_flag;   // Stop after w_num seconds flag
    int w_num;     // Stop after w_num seconds
    bool H_flag;   // Help flag
} t_flags;

typedef struct s_ping {
    char *binary_name;
    t_flags flags;
    char *destination_host;
    char ip[NI_MAXHOST * sizeof(char)]; // verificar el significado de este tamaño de array
    int socket_fd;
    struct sockaddr_in sa;
    struct in_addr inaddr;
} t_ping;

bool parsing(int argc, char *argv[], t_ping *ping);
int ft_ping(t_ping *ping);
void signal_handler(int signal);
bool convert_address(t_ping *ping);
void init_struct(t_ping *ping, char *string);
bool is_root(t_ping *ping);
bool dns_lookup(t_ping *ping);
bool create_raw_socket(t_ping *ping);
void send_ping(int ping_sockfd, struct sockaddr_in *ping_addr, char *ping_ip);
unsigned short checksum(void *b, int len);
int is_flag(const char *str);
bool get_flag_value(int argc, char **argv, t_ping *ping, int *i);

#endif
