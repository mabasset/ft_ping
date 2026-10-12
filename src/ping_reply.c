#include "ft_ping.h"

extern t_ping g_ping;

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

void handle_reply_error(struct iphdr* outer_iphdr,
                        t_icmp_packet* outer_icmp_packet) {
  struct iphdr inner_iphdr;
  t_icmp_packet inner_icmp_packet;

  if (get_packet_from_buffer(outer_icmp_packet->payload,
                             outer_icmp_packet->payload_len, &inner_iphdr,
                             &inner_icmp_packet))
    return;
}

void receive_echo_reply() {
  struct iphdr iphdr;
  t_icmp_packet icmp_packet;
  struct timeval recv_time;
  double rtt;

  if (get_packet_from_socket(&iphdr, &icmp_packet, &recv_time) < 0)
    return;
  else if (icmp_packet.hdr.un.echo.id != htons(getpid() & 0xFFFF))
    return;
  else if (icmp_packet.hdr.type != ICMP_ECHOREPLY) {
    if (g_ping.verbose)
      handle_reply_error(&iphdr, &icmp_packet);
    return;
  } else if (checksum(&icmp_packet,
                      sizeof(icmp_packet.hdr) + icmp_packet.payload_len) != 0)
    print_checksum_mismatch(&iphdr);
  g_ping.reply_count++;
  rtt = compute_rtt(&icmp_packet, recv_time);
  print_echo_reply(&iphdr, &icmp_packet, rtt);
  update_rtt_stats(rtt);
}