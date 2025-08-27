#include "ping.h"

// TODO incorrect first package when host unreachable ./ft_ping 10.0.2.230 -W 1 "64 bytes from 0.0.0.0: icmp_seq=0 ttl=0 time=1022.4 ms", se trato pero ahora no recibo mas timeout para icmp_seq 1
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

void get_timeout(const t_ping *ping, struct timeval *timeout) {
    timeout->tv_sec = ping->flags.W_flag == true ? ping->flags.W_num: RECV_TIMEOUT;
    timeout->tv_usec = 0;
}

void print_dest_unreach(char* sender_ip, const struct icmphdr* recv_hdr, const long unsigned int packet_received_size)
{
    switch (recv_hdr->code){
    case 1:
        printf("%lu bytes from %s: %s\n", packet_received_size, sender_ip, HOST_UNREACHABLE);
        break;
    case 0:
        printf("%lu bytes from %s: %s\n", packet_received_size, sender_ip, NET_UNREACHABLE);
        break;
    default:
        break;
    }
}

void print_first_line(t_ping* ping)
{
    if (ping->flags.v_flag){
        printf("PING %s (%s): %lu data bytes, id 0x%04x = %d\n", ping->destination_host, ping->ip,
               PING_PKT_S - sizeof(struct icmphdr), ping->ping_id, ping->ping_id);
    } else {
       printf("PING %s (%s): %lu data bytes\n", ping->destination_host, ping->ip, PING_PKT_S - sizeof(struct icmphdr));
    }
}

void print_packet_content(const struct iphdr* base_ip_hdr)
{
    const struct icmphdr *icmp_time_exceeded = (struct icmphdr *)((char *)base_ip_hdr + base_ip_hdr->ihl * 4);
    const struct iphdr *original_ip = (struct iphdr *)((char *)icmp_time_exceeded + 8);
    const struct icmphdr *original_icmp = (struct icmphdr *)((char *)original_ip + original_ip->ihl * 4);
    const unsigned char *byte = (unsigned char *)original_ip;

    char src_ip[INET_ADDRSTRLEN];
    char dst_ip[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, &original_ip->saddr, src_ip, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &original_ip->daddr, dst_ip, INET_ADDRSTRLEN);

    printf("IP Hdr Dump:\n ");
    for (int i = 0; i < original_ip->ihl * 4; i += 2) {
        printf(" %02x%02x", byte[i], byte[i+1]);
    }

    printf("\nVr HL TOS  Len   ID Flg  off TTL Pro  cks      Src\tDst\tData\n");
    printf(" %d  %d  %02x %04x %04x   %d %04x  %02x  %02x %04x %s  %s \n",
           original_ip->version, original_ip->ihl, original_ip->tos, ntohs(original_ip->tot_len), ntohs(original_ip->id),
           ntohs(original_ip->frag_off) >> 13 & 0x7, ntohs(original_ip->frag_off) & 0x1FFF, original_ip->ttl,
           original_ip->protocol, ntohs(original_ip->check), src_ip, dst_ip);
    printf("ICMP: type %d, code %d, size %d, id 0x%04x, seq 0x%04x\n",
           original_icmp->type, original_icmp->code, ntohs(original_ip->tot_len) - original_ip->ihl * 4,
           original_icmp->un.echo.id, original_icmp->un.echo.sequence);
}

void print_error_msg(const struct icmphdr* icmp_hdr) {
    switch (icmp_hdr->type){
        case ICMP_DEST_UNREACH:
            switch (icmp_hdr->code){
            case ICMP_NET_UNREACH:
                    printf("%s\n", NET_UNREACHABLE);
                    break;
            case ICMP_HOST_UNREACH:
                    printf("%s\n", HOST_UNREACHABLE);
                    break;
            case ICMP_PROT_UNREACH:
                    printf("%s\n", PROTOCOL_UNREACHABLE);
                    break;
            case ICMP_PORT_UNREACH:
                    printf("%s\n", PORT_UNREACHABLE);
                    break;
            case ICMP_FRAG_NEEDED:
                    printf("%s\n", FRAGMENTATION_NEEDED);
                    break;
            case ICMP_SR_FAILED:
                    printf("%s\n", SOURCE_ROUTE_FAILED);
                    break;
            case ICMP_NET_UNKNOWN:
                    printf("%s\n", NETWORK_UNKNOWN);
                    break;
            case ICMP_HOST_UNKNOWN:
                    printf("%s\n", HOST_UNKNOWN);
                    break;
            case ICMP_HOST_ISOLATED:
                    printf("%s\n", HOST_ISOLATED);
                    break;
            case ICMP_NET_ANO:
                    printf("%s\n", NETWORK_PROHIBITED);
                    break;
            case ICMP_HOST_ANO:
                    printf("%s\n", HOST_PROHIBITED);
                    break;
            case ICMP_NET_UNR_TOS:
                    printf("%s\n", NETWORK_UNREACHABLE_TOS);
                    break;
            case ICMP_HOST_UNR_TOS:
                    printf("%s\n", HOST_UNREACHABLE_TOS);
                    break;
            case ICMP_PKT_FILTERED:
                    printf("%s\n", PACKET_FILTERED);
                    break;
            case ICMP_PREC_VIOLATION:
                    printf("%s\n", PRECEDENCE_VIOLATION);
                    break;
            case ICMP_PREC_CUTOFF:
                    printf("%s\n", PRECEDENCE_CUTOFF);
                    break;
            default:
                    printf("Destination Unreachable (code %d)\n", icmp_hdr->code);
                    break;
            }
            break;
        case ICMP_SOURCE_QUENCH:
            printf("%s\n", SOURCE_QUENCH);
            break;
        case ICMP_REDIRECT:
            switch (icmp_hdr->code){
            case ICMP_REDIR_NET:
                    printf("%s\n", REDIRECT_NETWORK);
                    break;
            case ICMP_REDIR_HOST:
                    printf("%s\n", REDIRECT_HOST);
                    break;
            case ICMP_REDIR_NETTOS:
                    printf("%s\n", REDIRECT_NETWORK_TOS);
                    break;
            case ICMP_REDIR_HOSTTOS:
                    printf("%s\n", REDIRECT_HOST_TOS);
                    break;
            default:
                    printf("Redirect (code %d)\n", icmp_hdr->code);
                    break;
            }
            break;
        case ICMP_TIME_EXCEEDED:
            switch (icmp_hdr->code){
            case ICMP_EXC_TTL:
                    printf("%s\n", TTL_EXCEEDED);
                    break;
            case ICMP_EXC_FRAGTIME:
                    printf("%s\n", FRAGMENT_REASSEMBLY_TIME_EXCEEDED);
                    break;
            default:
                    printf("Time Exceeded (code %d)\n", icmp_hdr->code);
                    break;
            }
            break;
        case ICMP_PARAMETERPROB:
                printf("Parameter Problem (code %d)\n", icmp_hdr->code);
                break;
        case ICMP_TIMESTAMP:
            printf("%s\n", TIMESTAMP_REQUEST);
            break;
        case ICMP_TIMESTAMPREPLY:
            printf("%s\n", TIMESTAMP_REPLY);
            break;
        case ICMP_INFO_REQUEST:
            printf("%s\n", INFO_REQUEST);
            break;
        case ICMP_INFO_REPLY:
            printf("%s\n", INFO_REPLY);
            break;
        case ICMP_ADDRESS:
            printf("%s\n", ADDRESS_MASK_REQUEST);
            break;
        case ICMP_ADDRESSREPLY:
            printf("%s\n", ADDRESS_MASK_REPLY);
            break;
        default:
            printf("ICMP type %d, code %d\n", icmp_hdr->type, icmp_hdr->code);
            break;
    }
}

void print_non_echo_icmp(const t_ping *ping, const struct iphdr *base_ip_hdr, const struct icmphdr *icmp_hdr, char* sender_ip, const unsigned long received_packet_size)
{
    printf("%lu bytes from %s: ", received_packet_size, sender_ip);
    print_error_msg(icmp_hdr);
    if (ping->flags.v_flag) {
        print_packet_content(base_ip_hdr);
    }
}

void send_ping(t_ping *ping) {
    const int ttl = ping->flags.ttl_value != 0 ? ping->flags.ttl_value : DEFAULT_TTL;
    int msg_count = 0, addr_len, msg_received_count = 0, errors = 0;
    char recv_buffer[RECV_BUFFER_SIZE];
    struct ping_pkt packet;
    struct sockaddr_in recv_addr;
    struct timespec time_start_packet, time_end_packet, time_start_total;
    struct timeval timeout;

    ping->ping_id = getpid();

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

    print_first_line(ping);

    // Send ICMP packet in an infinite loop
    while (loop_running && !time_limit_reached(ping, time_start_total)) {
        bool packet_sent = true;
        fill_packet(&msg_count, &packet);

        clock_gettime(CLOCK_MONOTONIC, &time_start_packet);
        // Send packet
        if (sendto(ping->socket_fd, &packet, sizeof(packet), 0, (struct sockaddr*)&ping->sa, sizeof(ping->sa)) <= 0) {
            // fprintf(stderr, "Packet Sending Failed!\n");
            packet_sent = false;
        }

        // Receive packet
        memset(recv_buffer, 0, sizeof(recv_buffer));
        addr_len = (int) sizeof(recv_addr);
        memset(&recv_addr, 0, sizeof(recv_addr));

        size_t bytes_received = recvfrom(ping->socket_fd, recv_buffer, sizeof(recv_buffer), 0, (struct sockaddr*)&recv_addr, (socklen_t *) &addr_len);
        if (bytes_received == (size_t) -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // printf("Request timeout for icmp_seq %d\n", msg_count);
            } else {
                // fprintf(stderr, "Packet receive failed!\n");
                if (loop_running) {
                    errors++;
                }
            }
        } else {
            if (packet_sent) {
                char *sender_ip = inet_ntoa(recv_addr.sin_addr);
                const struct iphdr *ip_hdr = (struct iphdr *)recv_buffer;
                const struct icmphdr *icmp_hdr = (struct icmphdr *)(recv_buffer + ip_hdr->ihl * 4); // offset by ip header length
                const long unsigned int received_packet_size = bytes_received - sizeof(struct iphdr);

                if (icmp_hdr->type == ICMP_ECHOREPLY && icmp_hdr->code == 0 && icmp_hdr->un.echo.id == getpid()) {
                    const long double rtt = get_elapsed_time_ms(&time_start_packet, &time_end_packet);
                    process_rtt(&ping->rtt_list, rtt);
                    printf("%lu bytes from %s: icmp_seq=%d ttl=%d time=%.3Lf ms\n",
                        received_packet_size, sender_ip, icmp_hdr->un.echo.sequence, ip_hdr->ttl, get_last_rtt(ping->rtt_list));
                    msg_received_count++;
                } else {
                    print_non_echo_icmp(ping, ip_hdr, icmp_hdr, sender_ip, received_packet_size);
                    // errors++;
                }
            }
            if (!loop_running) {
                msg_count--;
            }
        }
        usleep(SLEEP_RATE);
    }

    // const long double total_elapsed_time = get_elapsed_time_ms(&time_start_total, &time_end_total);
    print_stats(ping, msg_count, msg_received_count, errors);
}

bool time_limit_reached(const t_ping *ping, const struct timespec time_start) {
    struct timespec time_end;

    if (ping->flags.w_flag){
        return get_elapsed_time_ms(&time_start, &time_end) / 1000 > ping->flags.w_num;
    }
    return false;
}

long double get_elapsed_time_ms(const struct timespec *time_start, struct timespec *time_end) {
    clock_gettime(CLOCK_MONOTONIC, time_end);
    const double nanoseconds_elapsed = ((double)(time_end->tv_nsec - (*time_start).tv_nsec)) / 1000000.0;
    const long double total_elapsed_time_ms = (time_end->tv_sec - time_start->tv_sec) * 1000.0 + nanoseconds_elapsed;
    return total_elapsed_time_ms;
}

void print_stats(const t_ping *ping, const int msg_count, const int msg_received_count, const int errors) {
    const double safe_divisor = msg_count > 0 ? msg_count : 1;
    char error_str[25] = "";

    if (errors > 0){
        sprintf(error_str, " +%d errors,", errors);
    }

    printf("--- %s ping statistics ---\n", ping->destination_host);
    printf("%d packets transmitted, %d packets received,%s %.0f%% packet loss\n", msg_count, msg_received_count,
           error_str, ((msg_count - msg_received_count) / safe_divisor) * 100.0);
    if (msg_received_count > 0){
        printf("round-trip min/avg/max/stddev = %.3Lf/%.3Lf/%.3Lf/%.3Lf ms\n", get_min_rtt(ping->rtt_list),
               get_avg_rtt(ping->rtt_list), get_max_rtt(ping->rtt_list), get_mdev_rtt(ping->rtt_list));
    }
    free_rtt_list(ping->rtt_list);
}

void fill_packet(int *msg_count, struct ping_pkt *packet) {
    int i;
    bzero(packet, sizeof(*packet));
    packet->icmp_header.type = ICMP_ECHO;
    packet->icmp_header.un.echo.id = getpid();

    for (i = 0; i < (int) sizeof(packet->msg) - 1; i++)
        packet->msg[i] = i + '0';

    packet->msg[i] = 0;
    packet->icmp_header.un.echo.sequence = (*msg_count)++;
    packet->icmp_header.checksum = checksum(packet, sizeof(*packet));
}

// RFC 1071
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