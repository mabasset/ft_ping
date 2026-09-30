#include "ft_ping.h"

extern t_flags g_flags;

int resolve_target(const char* hostname, t_target* target) {
  struct addrinfo hints = {0};
  struct addrinfo* res;

  // the only hints fileds that getaddrinfo checks
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_RAW;
  hints.ai_protocol = IPPROTO_ICMP;
  hints.ai_flags = 0;

  /* puts in res->ai_addr a list of valid ips based on the hostname and filtered
   * by hints
   */
  if (getaddrinfo(hostname, NULL, &hints, &res) != 0)
    return -1;

  target->hostname = hostname;
  /* sockaddr is a generic type that must be cast based of the ip type(ipv4 or
   * ipv6)
   */
  target->addr = *(struct sockaddr_in*)res->ai_addr;
  // translate addr.sin_addr into a readable string target->ip
  inet_ntop(AF_INET, &target->addr.sin_addr, target->ip, sizeof(target->ip));

  freeaddrinfo(res);
  return 0;
}

void build_echo_request(t_icmp_packet* packet, uint16_t seq) {
  memset(packet, 0, sizeof(*packet));

  packet->hdr.type = ICMP_ECHO;
  packet->hdr.code = 0;
  packet->hdr.un.echo.id = htons(getpid() & 0xFFFF);
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
