#include "ping.h"

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
