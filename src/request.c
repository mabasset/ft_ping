#include "ft_ping.h"

extern t_flags g_flags;

// RFC 1071 Internet checksum
static uint16_t checksum(void* data, int len) {
  uint16_t* buf = data;
  uint32_t sum = 0;

  for (; len > 1; len -= 2)
    sum += *buf++;
  if (len == 1)
    sum += *(uint8_t*)buf;

  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  return (uint16_t)~sum;
}

void build_echo_request(t_icmp_packet* packet, uint16_t seq) {
  memset(packet, 0, sizeof(*packet));

  packet->hdr.type = ICMP_ECHO;
  packet->hdr.code = 0;
  packet->hdr.un.echo.id = htons(getuid());
  packet->hdr.un.echo.sequence = htons(seq);

  size_t i = 0;
  if (g_flags.size >= sizeof(struct timeval)) {
    i = sizeof(struct timeval);
    gettimeofday((struct timeval*)packet->payload, NULL);
  }

  while (i < g_flags.size) {
    packet->payload[i] = (char)i;
    i++;
  }

  packet->hdr.checksum = 0;
  packet->hdr.checksum = checksum(packet, sizeof(packet->hdr) + g_flags.size);
}