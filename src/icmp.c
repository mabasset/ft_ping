#include "ft_ping.h"

extern t_ping g_ping;

void build_echo_request(t_icmp_packet* packet, uint16_t seq) {
  memset(packet, 0, sizeof(*packet));

  packet->hdr.type = ICMP_ECHO;
  packet->hdr.code = 0;
  packet->hdr.un.echo.id = htons(getpid() & 0xFFFF);
  packet->hdr.un.echo.sequence = htons(seq);

  size_t i = 0;
  if (g_ping.payload_size >= sizeof(struct timeval)) {
    i = sizeof(struct timeval);
    gettimeofday((struct timeval*)packet->payload, NULL);
  }

  while (i < g_ping.payload_size) {
    packet->payload[i] = (char)i;
    i++;
  }

  packet->hdr.checksum = 0;
  packet->hdr.checksum =
      checksum(packet, sizeof(packet->hdr) + g_ping.payload_size);
}
