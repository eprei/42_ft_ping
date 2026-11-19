#include "ping.h"

volatile sig_atomic_t loop_running = true;

static void loop(t_config *config) {
    const int ttl = config->flags.ttl_value != 0 ? config->flags.ttl_value : DEFAULT_TTL;
    int msgs_sent = 0, addr_len, msgs_received = 0, errors = 0;
    char buf[BUF_SIZE];
    struct ping_pkt packet;
    struct sockaddr_in recv_addr;
    struct timespec time_start_total;
    struct timeval next_send, now, wait, sent, result;
    fd_set set;

    clock_gettime(CLOCK_MONOTONIC, &time_start_total);
    // Set socket options IP TTL
    if (setsockopt(config->socket_fd, SOL_IP, IP_TTL, &ttl, sizeof(ttl)) != 0) {
        fprintf(stderr, "Setting socket options to TTL failed!\n");
        return;
    }

    print_first_line(config);

    // Send ICMP packet in a loop
    gettimeofday(&next_send, NULL);
    while (loop_running && !is_time_limit_reached(config, time_start_total)) {
        gettimeofday(&now, NULL);

        if (timercmp(&now, &next_send, >=)) {
            fill_packet(&msgs_sent, &packet, config->pid);
            if (sendto(config->socket_fd, &packet, sizeof(packet), 0, (struct sockaddr*)&config->sa, sizeof(config->sa)) <= 0) {
                perror("sendto");
            }
            next_send = now;
            next_send.tv_sec += 1;
        }

        timersub(&next_send, &now, &wait);
        FD_ZERO(&set);
        FD_SET(config->socket_fd, &set);

        if (select(config->socket_fd + 1, &set, NULL, NULL, &wait) > 0) {
            memset(buf, 0, sizeof(buf));
            addr_len = (int) sizeof(recv_addr);
            memset(&recv_addr, 0, sizeof(recv_addr));
            const size_t bytes_received = recvfrom(config->socket_fd, buf, sizeof(buf), 0, (struct sockaddr*)&recv_addr, (socklen_t *) &addr_len);
            if (bytes_received == (size_t) -1) {
                if (errno != EAGAIN && errno != EWOULDBLOCK && loop_running) {
                    errors++;
                }
            } else {
                char *sender_ip = inet_ntoa(recv_addr.sin_addr);
                const struct iphdr *ip_hdr = (struct iphdr *)buf;
                const struct icmphdr *icmp_hdr = (struct icmphdr *)(buf + ip_hdr->ihl * 4); // offset by ip header length
                const long unsigned int received_packet_size = bytes_received - sizeof(struct iphdr);

                if (icmp_hdr->type == ICMP_ECHOREPLY && icmp_hdr->code == 0 && icmp_hdr->un.echo.id == config->pid) {
                    memcpy(&sent, (char *)icmp_hdr + sizeof(struct icmphdr), sizeof(sent));
                    gettimeofday(&now, NULL);

                    timersub(&now, &sent, &result);
                    const long double rtt = timeval_to_rtt(&result);
                    process_rtt(&config->rtt_list, rtt);
                    printf("%lu bytes from %s: icmp_seq=%d ttl=%d time=%.3Lf ms\n",
                        received_packet_size, sender_ip, icmp_hdr->un.echo.sequence, ip_hdr->ttl, get_last_rtt(config->rtt_list));
                    msgs_received++;
                } else {
                    print_non_echo_icmp(config, ip_hdr, icmp_hdr, sender_ip, received_packet_size);
                }
                if (!loop_running) {
                    msgs_sent--;
                }
            }
        }
    }

    print_stats(config, msgs_sent, msgs_received, errors);
}

int ping(t_config *config) {
    if (convert_address(config) || dns_lookup(config)){
        loop(config);
        return 0;
    }
    return 1;
}