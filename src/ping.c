#include "ping.h"

volatile sig_atomic_t loop_running = 1;

int ft_ping(t_ping *ping) {
    if (convert_address(ping) || dns_lookup(ping)){
          ping_loop(ping);
        return 0;
    }
    return 1;
}

void ping_loop(t_ping *ping) {
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
    while (loop_running && !is_time_limit_reached(ping, time_start_total)) {
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
                printf("Request timeout for icmp_seq %d\n", msg_count);
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
