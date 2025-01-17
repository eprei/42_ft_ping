#include "ft_ping.h"

volatile sig_atomic_t loop_running = 1;

int ft_ping(t_ping *ping) {
    if (convert_address(ping) ||  dns_lookup(ping)){
          send_ping(ping->socket_fd, &ping->sa, ping->ip);
        return 0;
    }

    return 1;
}

bool convert_address(t_ping *ping) {
    int convert_address_result = inet_pton(AF_INET, ping->destination_host, &ping->inaddr);
   
    if (convert_address_result <= 0) {
        if (convert_address_result == 0) {
            fprintf(stderr, "Not in presentation format");
        }
        else {
            perror("inet_pton");
        }
        return false;
    }
    return true;
}

bool dns_lookup(t_ping *ping) {
    printf("\nResolving DNS...\n");

    struct hostent *host_entity;
    if ((host_entity = gethostbyname(ping->destination_host)) == NULL) {
        fprintf(stderr, UNKNOWN_HOST, ping->binary_name, ping->ip);
        return false;
    }

    // Fill up address structure
    strcpy(ping->ip, inet_ntoa(*(struct in_addr *)host_entity->h_addr));
    ping->sa.sin_family = host_entity->h_addrtype;
    ping->sa.sin_port = htons(PORT_NO);
    ping->sa.sin_addr.s_addr = *(long *)host_entity->h_addr;

    return true;
}

void send_ping(int ping_sockfd, struct sockaddr_in *ping_addr, char *ping_ip) {
    int ttl_val = 64, msg_count = 0, i, addr_len, flag = 1, msg_received_count = 0;
    char rbuffer[128];
    struct ping_pkt pckt;
    struct sockaddr_in r_addr;
    struct timespec time_start, time_end, tfs, tfe;
    long double rtt_msec = 0, total_msec = 0;
    struct timeval tv_out;
    tv_out.tv_sec = RECV_TIMEOUT;
    tv_out.tv_usec = 0;

    clock_gettime(CLOCK_MONOTONIC, &tfs);

    // Set socket options at IP to TTL and value to 64
    if (setsockopt(ping_sockfd, SOL_IP, IP_TTL, &ttl_val, sizeof(ttl_val)) != 0) {
//        printf("\nSetting socket options to TTL failed!\n");
        return;
    } else {
//        printf("\nSocket set to TTL...\n");
    }

    // Setting timeout of receive setting
    setsockopt(ping_sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_out, sizeof tv_out);

    // print correctly PING GOOGLE...
    printf("%d packets transmited, %d received, %f%% packet loss, time %Lf ms\n", msg_count, msg_received_count, ((msg_count - msg_received_count) / (double)msg_count) * 100.0, total_msec);

    // Send ICMP packet in an infinite loop
    while (loop_running) {
        // Flag to check if packet was sent or not
        flag = 1;

        // Fill the packet
        bzero(&pckt, sizeof(pckt));
        pckt.hdr.type = ICMP_ECHO;
        pckt.hdr.un.echo.id = getpid();

        for (i = 0; i < (int) sizeof(pckt.msg) - 1; i++)
            pckt.msg[i] = i + '0';

        pckt.msg[i] = 0;
        pckt.hdr.un.echo.sequence = msg_count++;
        pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));

        usleep(PING_SLEEP_RATE);

        // Send packet
        clock_gettime(CLOCK_MONOTONIC, &time_start);
        if (sendto(ping_sockfd, &pckt, sizeof(pckt), 0, (struct sockaddr*)ping_addr, sizeof(*ping_addr)) <= 0) {
            printf("\nPacket Sending Failed!\n");
            flag = 0;
        }

        // Receive packet
        addr_len = (int) sizeof(r_addr);
        if (recvfrom(ping_sockfd, rbuffer, sizeof(rbuffer), 0, (struct sockaddr*)&r_addr, (socklen_t *) &addr_len) <= 0 && msg_count > 1) {
            printf("\nPacket receive failed!\n");
        } else {
            clock_gettime(CLOCK_MONOTONIC, &time_end);

            double timeElapsed = ((double)(time_end.tv_nsec - time_start.tv_nsec)) / 1000000.0;
            rtt_msec = (time_end.tv_sec - time_start.tv_sec) * 1000.0 + timeElapsed;

            // If packet was not sent, don't receive
            if (flag) {
                struct icmphdr *recv_hdr = (struct icmphdr *)rbuffer;
                if (!(recv_hdr->type == 0 && recv_hdr->code == 0)) {
                    printf("Error... Packet received with ICMP type %d code %d\n", recv_hdr->type, recv_hdr->code);
                } else {
                    printf("%d bytes from %s (h: %s) (ip: %s) msg_seq = %d ttl = %d rtt = %Lf ms.\n", PING_PKT_S, "TO_FULLFILL_WITH_RIGHT_VARIABLE", ping_ip, ping_ip, msg_count, ttl_val, rtt_msec);
                    msg_received_count++;
                }
            }
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &tfe);
    double timeElapsed = ((double)(tfe.tv_nsec - tfs.tv_nsec)) / 1000000.0;
    total_msec = (tfe.tv_sec - tfs.tv_sec) * 1000.0 + timeElapsed;

    printf("--- %s ping statistics ---\n", ping_ip);
    printf("%d packets transmited, %d received, %f%% packet loss, time %Lf ms\n", msg_count, msg_received_count, ((msg_count - msg_received_count) / (double)msg_count) * 100.0, total_msec);
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