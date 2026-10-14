#include "ft_ping.h"

extern t_ping g_ping;

static void print_checksum_mismatch(const struct iphdr* iphdr) {
  char replier_ip[INET_ADDRSTRLEN];

  inet_ntop(AF_INET, &iphdr->saddr, replier_ip, sizeof(replier_ip));
  printf("checksum mismatch from %s\n", replier_ip);
}

static void print_reply(const struct iphdr* iphdr,
                        const t_icmp_packet* icmp_packet,
                        double rtt) {
  uint16_t sequence;
  char replier_ip[INET_ADDRSTRLEN];

  sequence = ntohs(icmp_packet->hdr.un.echo.sequence);
  inet_ntop(AF_INET, &iphdr->saddr, replier_ip, sizeof(replier_ip));
  if (rtt != 0) {
    printf("%zu bytes from %s: icmp_seq=%u ttl=%u time=%.3f ms\n",
           icmp_packet->payload_len + sizeof(struct icmphdr), replier_ip,
           sequence, iphdr->ttl, rtt);
  } else {
    printf("%zu bytes from %s: icmp_seq=%u ttl=%u\n",
           icmp_packet->payload_len + sizeof(struct icmphdr), replier_ip,
           sequence, iphdr->ttl);
  }
}

static void update_rtt_stats(double rtt) {
  if (rtt == 0)
    return;
  if (rtt > g_ping.rtt_max)
    g_ping.rtt_max = rtt;
  if (rtt < g_ping.rtt_min || g_ping.rtt_min == 0)
    g_ping.rtt_min = rtt;
  g_ping.rtt_sum += rtt;
  g_ping.rtt_sum_sq += rtt * rtt;
}

static double compute_rtt(const t_icmp_packet* icmp_packet,
                          struct timeval recv_time) {
  struct timeval send_time;
  double rtt;

  if (icmp_packet->payload_len < sizeof(struct timeval))
    return 0;
  memcpy(&send_time, icmp_packet->payload, sizeof(send_time));
  rtt = timeval_to_ms(recv_time) - timeval_to_ms(send_time);

  return rtt;
}

static int get_packet_from_buffer(const char* buffer,
                                  ssize_t buffer_len,
                                  struct iphdr* iphdr,
                                  t_icmp_packet* icmp_packet) {
  size_t iphdr_len;

  if (buffer_len < 0 || (size_t)buffer_len < sizeof(struct iphdr))
    return -1;
  *iphdr = *(struct iphdr*)buffer;
  iphdr_len = iphdr->ihl * 4;
  if ((size_t)buffer_len < iphdr_len + sizeof(struct icmphdr))
    return -1;
  icmp_packet->hdr = *(struct icmphdr*)(buffer + iphdr_len);
  icmp_packet->payload_len = buffer_len - iphdr_len - sizeof(struct icmphdr);
  if (icmp_packet->payload_len > PING_MAX_PAYLOAD_SIZE)
    icmp_packet->payload_len = PING_MAX_PAYLOAD_SIZE;
  memcpy(icmp_packet->payload, buffer + iphdr_len + sizeof(struct icmphdr),
         icmp_packet->payload_len);

  return 0;
}

static int get_packet_from_socket(struct iphdr* iphdr,
                                  t_icmp_packet* icmp_packet,
                                  struct timeval* recv_time) {
  char buffer[MAX_IPHDR_SIZE + sizeof(struct icmphdr) + PING_MAX_PAYLOAD_SIZE];
  ssize_t n_bytes;

  n_bytes = recv(g_ping.sockfd, buffer, sizeof(buffer), 0);
  gettimeofday(recv_time, NULL);

  return get_packet_from_buffer(buffer, n_bytes, iphdr, icmp_packet);
}

static int check_packet(struct iphdr* outer_iphdr,
                        t_icmp_packet* outer_icmp_packet) {
  struct iphdr inner_iphdr;
  t_icmp_packet inner_icmp_packet;

  if (outer_icmp_packet->hdr.type == ICMP_ECHO)
    return -1;
  else if (outer_icmp_packet->hdr.type != ICMP_ECHOREPLY) {
    if (g_ping.verbose &&
        get_packet_from_buffer(outer_icmp_packet->payload,
                               outer_icmp_packet->payload_len, &inner_iphdr,
                               &inner_icmp_packet) == 0) {
      if (inner_icmp_packet.hdr.un.echo.id != htons(getpid() & 0xFFFF))
        return 0;
      handle_reply_error(outer_iphdr, outer_icmp_packet, &inner_iphdr,
                         &inner_icmp_packet);
    }
    g_ping.error_count++;
    return -1;
  } else if (outer_icmp_packet->hdr.un.echo.id != htons(getpid() & 0xFFFF))
    return -1;
  else if (checksum(outer_icmp_packet, sizeof(outer_icmp_packet->hdr) +
                                           outer_icmp_packet->payload_len) != 0)
    print_checksum_mismatch(outer_iphdr);

  return 0;
}

void receive_reply() {
  struct iphdr iphdr;
  t_icmp_packet icmp_packet;
  struct timeval recv_time;
  double rtt;

  if (get_packet_from_socket(&iphdr, &icmp_packet, &recv_time) < 0)
    return;
  else if (check_packet(&iphdr, &icmp_packet) < 0)
    return;
  g_ping.reply_count++;
  rtt = compute_rtt(&icmp_packet, recv_time);
  print_reply(&iphdr, &icmp_packet, rtt);
  update_rtt_stats(rtt);
}