#include "ping.h"

void print_first_line(t_config* ping){
    if (ping->flags.v_flag){
        printf("PING %s (%s): %lu data bytes, id 0x%04x = %d\n", ping->destination_host, ping->ip,
               PING_PKT_S - sizeof(struct icmphdr), ping->pid, ping->pid);
    } else {
       printf("PING %s (%s): %lu data bytes\n", ping->destination_host, ping->ip, PING_PKT_S - sizeof(struct icmphdr));
    }
}

void print_packet_content(const struct iphdr* base_ip_hdr){
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
        printf("%02x%02x ", byte[i], byte[i+1]);
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

void print_custom_message(const struct icmphdr* icmp_hdr) {
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
        default:
            printf("ICMP type %d, code %d\n", icmp_hdr->type, icmp_hdr->code);
            break;
    }
}

void print_non_echo_icmp(const t_config *ping, const struct iphdr *base_ip_hdr, const struct icmphdr *icmp_hdr, char* sender_ip, const unsigned long received_packet_size){
    printf("%lu bytes from %s: ", received_packet_size, sender_ip);
    print_custom_message(icmp_hdr);
    if (ping->flags.v_flag) {
        print_packet_content(base_ip_hdr);
    }
}

void print_stats(const t_config *ping, const int msgs_sent, const int msgs_received, const int errors) {
    const double safe_divisor = msgs_sent > 0 ? msgs_sent : 1;
    char error_str[25] = "";

    if (errors > 0){
        sprintf(error_str, " +%d errors,", errors);
    }

    printf("--- %s ping statistics ---\n", ping->destination_host);
    printf("%d packets transmitted, %d packets received,%s %.0f%% packet loss\n", msgs_sent, msgs_received,
           error_str, (msgs_sent - msgs_received) / safe_divisor * 100.0);
    if (msgs_received > 0){
        printf("round-trip min/avg/max/stddev = %.3Lf/%.3Lf/%.3Lf/%.3Lf ms\n", get_min_rtt(ping->rtt_list),
               get_avg_rtt(ping->rtt_list), get_max_rtt(ping->rtt_list), get_mdev_rtt(ping->rtt_list));
    }
    free_rtt_list(ping->rtt_list);
}

void print_option_need_argument(const int flag_value){
    char option[6];

    switch (flag_value){
        case TTL_FLAG:
            strcpy(option, "--ttl");
            break;
        case W_FLAG:
            strcpy(option, "W");
            break;
        case w_FLAG:
            strcpy(option, "w");
            break;
        default:
            strcpy(option, " ");
            break;
    }
    fprintf(stderr, "ft_ping: option requires an argument -- '%s'\n", option);
    fprintf(stderr, "%s\n", MSG_HELP);
}

void print_usage(void) {
    fprintf(stderr, "%s\n", USAGE_MSG);
}

void print_error(char *error_msg){
    fprintf(stderr, "%s\n", error_msg);
}

//TODO resolve
// sudo ./ft_ping 8.8.8.8 --ttl
// ft_ping: invalid option -- '-'
// MUST BE
// ping 8.8.8.8 --ttl
// ping: option '--ttl' requires an argument