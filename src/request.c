#include "ft_ping.h"

extern t_ping g_ping;

static void build_request(t_icmp_packet* packet, struct timeval* send_time) {
  memset(packet, 0, sizeof(*packet));
  packet->hdr.type = ICMP_ECHO;
  packet->hdr.code = 0;
  packet->hdr.un.echo.id = htons(g_ping.pid);
  packet->hdr.un.echo.sequence = htons((uint16_t)g_ping.request_count);
  size_t i = 0;
  if (g_ping.payload_size >= sizeof(struct timeval)) {
    i = sizeof(struct timeval);
    memcpy(packet->payload, send_time, sizeof(struct timeval));
  }
  while (i < g_ping.payload_size) {
    packet->payload[i] = (char)i;
    i++;
  }
  packet->hdr.checksum = 0;
  packet->hdr.checksum =
      checksum(packet, sizeof(packet->hdr) + g_ping.payload_size);
}

void send_request(t_icmp_packet* packet,
                  struct timeval* send_time,
                  struct sockaddr_in addr) {
  if (g_ping.request_limit_reached)
    return;
  gettimeofday(send_time, NULL);
  build_request(packet, send_time);
  if (sendto(g_ping.sockfd, packet, sizeof(packet->hdr) + g_ping.payload_size,
             0, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    perror("ping: sending packet");
    exit(1);
  }
  bitmap_clear(g_ping.seq_bitmap, (uint16_t)g_ping.request_count);
  g_ping.request_count++;
  if (g_ping.packet_limit == g_ping.request_count)
    g_ping.request_limit_reached = true;
}
