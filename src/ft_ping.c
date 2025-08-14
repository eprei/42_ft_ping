#include "ft_ping.h"

volatile sig_atomic_t loop_running = 1;

int ft_ping(t_ping *ping) {
    if (convert_address(ping) ||  dns_lookup(ping)){
          send_ping(ping);
        return 0;
    }

    return 1;
}

bool convert_address(t_ping *ping) {
    int convert_address_result = inet_pton(AF_INET, ping->destination_host, &ping->sa.sin_addr);
   
    if (convert_address_result <= 0) {
        if (convert_address_result == 0) {
//            fprintf(stderr, "Not in presentation format");
        }
        else {
            perror("inet_pton");
        }
        return false;
    }

    strcpy(ping->ip, ping->destination_host);

    return true;
}

bool dns_lookup(t_ping *ping) {
    struct hostent *host_entity;
    if ((host_entity = gethostbyname(ping->destination_host)) == NULL) {
        fprintf(stderr, UNKNOWN_HOST_MSG, ping->binary_name, ping->ip);
        return false;
    }

    // Fill up address structure
    strcpy(ping->ip, inet_ntoa(*(struct in_addr *)host_entity->h_addr));
    ping->sa.sin_family = host_entity->h_addrtype;

    return true;
}

void send_ping(t_ping *ping) {
    int ttl_val = ping->flags.ttl_value != 0 ? ping->flags.ttl_value : DEFAULT_TTL;
    int msg_count = 1, addr_len, msg_received_count = 0;
    char rbuffer[128]; // TODO 128 magic number ?
    struct ping_pkt pckt;
    struct sockaddr_in r_addr;
    struct timespec time_start, time_end, tfs, tfe;
    long double rtt = 0, avg, max, total_msec = 0;
    t_rtt *rtt_chained = NULL;
    struct timeval tv_out;
    tv_out.tv_sec = RECV_TIMEOUT;
    tv_out.tv_usec = 0;

    clock_gettime(CLOCK_MONOTONIC, &tfs);

    // Set socket options at IP to TTL and value to 64
    if (setsockopt(ping->socket_fd, SOL_IP, IP_TTL, &ttl_val, sizeof(ttl_val)) != 0) {
        fprintf(stderr, "Setting socket options to TTL failed!\n");
        return;
    } else {
//        printf("\nSocket set to TTL...\n");
    }

    // Setting timeout of receive setting
    setsockopt(ping->socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_out, sizeof tv_out);

    printf("PING %s (%s) 56(84) bytes of data.\n", ping->destination_host, ping->ip); // TODO get size pack dynamically ?

    // Send ICMP packet in an infinite loop
    while (loop_running) {
        bool packet_sent = true;

        fill_packet(&msg_count, &pckt);
        usleep(SLEEP_RATE);

        // Send packet
        clock_gettime(CLOCK_MONOTONIC, &time_start);
        if (sendto(ping->socket_fd, &pckt, sizeof(pckt), 0, (struct sockaddr*)&ping->sa, sizeof(ping->sa)) <= 0) {
            fprintf(stderr, "Packet Sending Failed!\n");
            packet_sent = false;
        }

        // Receive packet
        addr_len = (int) sizeof(r_addr);
        if (recvfrom(ping->socket_fd, rbuffer, sizeof(rbuffer), 0, (struct sockaddr*)&r_addr, (socklen_t *) &addr_len) <= 0 && msg_count > 1) {
            fprintf(stderr, "Packet receive failed!\n");
        } else {
            clock_gettime(CLOCK_MONOTONIC, &time_end);

            double timeElapsed = ((double)(time_end.tv_nsec - time_start.tv_nsec)) / 1000000.0;
            rtt = (time_end.tv_sec - time_start.tv_sec) * 1000.0 + timeElapsed;

            //TODO if a reply is not received within in a certain timeframe, you'll see a "Request timed out"
            if (packet_sent) {
                struct iphdr *ip_hdr = (struct iphdr *)rbuffer;
                struct icmphdr *recv_hdr = (struct icmphdr *)(rbuffer + (ip_hdr->ihl * 4));

                if (recv_hdr->type == ICMP_ECHOREPLY && recv_hdr->code == 0) {
                    printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.1Lf ms\n",
                           PING_PKT_S, ping->ip, recv_hdr->un.echo.sequence, ip_hdr->ttl, rtt); //TODO rever esta impresion pq los datos no son correctos
                    msg_received_count++; //TODO PING_PKT_S deberia ser substituido por el tamano del ICMP echo reply recivido
                }
            }
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &tfe);
    double timeElapsed = ((double)(tfe.tv_nsec - tfs.tv_nsec)) / 1000000.0;
    total_msec = (tfe.tv_sec - tfs.tv_sec) * 1000.0 + timeElapsed;

    printf("--- %s ping statistics ---\n", ping->destination_host);
    printf("%d packets transmitted, %d received, %.0f%% packet loss, time %.0Lfms\n", msg_count, msg_received_count, ((msg_count - msg_received_count) / (double)msg_count) * 100.0, total_msec); // TODO corregir cifras significativas
    printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n", 0.1, 0.2, 0.3, 0.4); // TODO printf rtt min/avg/max/mdev = 13.071/13.071/13.071/0.000 ms
}

void fill_packet(int *msg_count, struct ping_pkt *pckt) {
    int i;
    bzero(pckt, sizeof(*pckt));
    (*pckt).icmp_header.type = ICMP_ECHO;
    (*pckt).icmp_header.un.echo.id = getpid();

    for (i = 0; i < (int) sizeof((*pckt).msg) - 1; i++)
        (*pckt).msg[i] = i + '0';

    (*pckt).msg[i] = 0;
    (*pckt).icmp_header.un.echo.sequence = (*msg_count)++;
    (*pckt).icmp_header.checksum = checksum(pckt, sizeof(*pckt));
}

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