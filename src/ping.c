#include "ping.h"

// TODO incorrect behaviour when using dns
// TODO incorrect first package when host unreachable ./ft_pint 10.0.2.230 -W 1 "64 bytes from 0.0.0.0: icmp_seq=0 ttl=0 time=1022.4 ms", se trato pero ahora no recibo mas timeout para icmp_seq 1
// TODO respecto al renglonnnnnnn aanteeerior, si el host es una direccion valida de nuestra red pero inexistente, no hay error. si el host es cualquier cosa no funciona
// TODO cuando no se recibe nada, el verdadero ping no imprime nada, ni siquiera el timeout, el primer paquete perdido es algo     y no se que

volatile sig_atomic_t loop_running = 1;

int ft_ping(t_ping *ping) {
    if (convert_address(ping) ||  dns_lookup(ping)){
          send_ping(ping);
        return 0;
    }
    return 1;
}

void get_timeout(t_ping *ping, struct timeval *timeout) {
    timeout->tv_sec = ping->flags.W_flag == true ? ping->flags.W_num: RECV_TIMEOUT;
    timeout->tv_usec = 0;
}

void send_ping(t_ping *ping) {
    int ttl = ping->flags.ttl_value != 0 ? ping->flags.ttl_value : DEFAULT_TTL;
    int msg_count = 0, addr_len, msg_received_count = 0, errors = 0;
    char rbuffer[128]; // TODO 128 magic number ?
    struct ping_pkt pckt;
    struct sockaddr_in r_addr;
    struct timespec time_start_pkt, time_end_pkt, time_start_total, time_end_total;
    long double total_elapsed_time;
    struct timeval timeout;

    clock_gettime(CLOCK_MONOTONIC, &time_start_total);
    // Set socket options at IP to TTL and value to 64
    if (setsockopt(ping->socket_fd, SOL_IP, IP_TTL, &ttl, sizeof(ttl)) != 0) {
        fprintf(stderr, "Setting socket options to TTL failed!\n");
        return;
    } else {
//        printf("\nSocket set to TTL...\n");
    }

    // Setting timeout of receive setting
    get_timeout(ping, &timeout);
    setsockopt(ping->socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof timeout);

    printf("PING %s (%s) 56(84) bytes of data.\n", ping->destination_host, ping->ip); // TODO get size pack dynamically ?

    // Send ICMP packet in an infinite loop
    while (loop_running && !time_limit_reached(ping, time_start_total)) {
        bool packet_sent = true;
        fill_packet(&msg_count, &pckt);

        clock_gettime(CLOCK_MONOTONIC, &time_start_pkt);
        // Send packet
        if (sendto(ping->socket_fd, &pckt, sizeof(pckt), 0, (struct sockaddr*)&ping->sa, sizeof(ping->sa)) <= 0) {
            fprintf(stderr, "Packet Sending Failed!\n");
            packet_sent = false;
        }

        // Receive packet
        memset(rbuffer, 0, sizeof(rbuffer));
        addr_len = (int) sizeof(r_addr);
        memset(&r_addr, 0, sizeof(r_addr));
        if (recvfrom(ping->socket_fd, rbuffer, sizeof(rbuffer), 0, (struct sockaddr*)&r_addr, (socklen_t *) &addr_len) <= 0 && msg_count > 1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                printf("Request timeout for icmp_seq %d\n", msg_count);
            } else {
//                fprintf(stderr, "Packet receive failed!\n");
            }
            errors++;
        } else {
            long double rtt = elapsed_time_ms(&time_start_pkt, &time_end_pkt);
            process_rtt(&ping->rtt_list, rtt);
            printf("recibido %d\n", msg_count);
            if (packet_sent) {
                char *sender_ip = inet_ntoa(r_addr.sin_addr);
                struct iphdr *ip_hdr = (struct iphdr *)rbuffer;
                struct icmphdr *recv_hdr = (struct icmphdr *)(rbuffer + (ip_hdr->ihl * 4)); //TODO ver el pq de este offset

                if (recv_hdr->type == ICMP_ECHOREPLY && recv_hdr->code == 0 && recv_hdr->un.echo.id == getpid()) {
                        printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.1Lf ms\n",
                               PING_PKT_S, sender_ip, recv_hdr->un.echo.sequence, ip_hdr->ttl, get_last_rtt(ping->rtt_list)); //TODO PING_PKT_S deberia ser substituido por el tamano del ICMP echo reply recivido && valor TTL no es igual al del ping real
                } else if (recv_hdr->type == ICMP_DEST_UNREACH && recv_hdr->code == 1){
                    printf("From %s icmp_seq=%d %s\n", sender_ip, msg_count, HOST_UNREACHABLE);
                    errors++;
                }
                msg_received_count++; // TODO donde va???
            }
//            if (!loop_running) {
//                msg_count--;
//            }
        }
        usleep(SLEEP_RATE);
    }

    total_elapsed_time = elapsed_time_ms(&time_start_total, &time_end_total);
    print_statistics(ping, msg_count, msg_received_count, total_elapsed_time, errors);
}

bool time_limit_reached(t_ping *ping, struct timespec time_start) {
    struct timespec time_end;

    if (ping->flags.w_flag){
        return elapsed_time_ms(&time_start, &time_end) / 1000 > ping->flags.w_num;
    }
    return false;
}

long double elapsed_time_ms(struct timespec *time_start, struct timespec *time_end) {
    long double total_elapsed_time_ms;
    clock_gettime(CLOCK_MONOTONIC, time_end);
    double nanoseconds_elapsed = ((double)((*time_end).tv_nsec - (*time_start).tv_nsec)) / 1000000.0;
    total_elapsed_time_ms = ((*time_end).tv_sec - (*time_start).tv_sec) * 1000.0 + nanoseconds_elapsed;
    return total_elapsed_time_ms;
}

void print_statistics(const t_ping *ping, int msg_count, int msg_received_count, long double total_time, int errors) {
    double safe_divisor = msg_count > 0 ? msg_count : 1;
    char error_str[25] = "";

    if (errors != 0){
        sprintf(error_str, " +%d errors,", errors);
    }

    printf("--- %s ping statistics ---\n", ping->destination_host);
    printf("%d packets transmitted, %d received,%s %.0f%% packet loss, time %.0Lfms\n", msg_count, msg_received_count,
           error_str, ((msg_count - msg_received_count) / safe_divisor) * 100.0, total_time);
    printf("rtt min/avg/max/mdev = %.3Lf/%.3Lf/%.3Lf/%.3Lf ms\n", get_min_rtt(ping->rtt_list),
           get_avg_rtt(ping->rtt_list), get_max_rtt(ping->rtt_list), get_mdev_rtt(ping->rtt_list));

    free_rtt_list(ping->rtt_list);
}

void fill_packet(int *msg_count, struct ping_pkt *pckt) {
    int i;
    bzero(pckt, sizeof(*pckt));
    (*pckt).icmp_header.type = ICMP_ECHO;
    (*pckt).icmp_header.un.echo.id = getpid();

    for (i = 0; i < (int) sizeof((*pckt).msg) - 1; i++)
        (*pckt).msg[i] = i + '0';

    (*pckt).msg[i] = 0;
    (*pckt).icmp_header.un.echo.sequence = ++(*msg_count);
    (*pckt).icmp_header.checksum = checksum(pckt, sizeof(*pckt));
    printf("paquet %d\n", *msg_count);
}

//TODO review this function
unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}