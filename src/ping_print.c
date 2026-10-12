#include "ft_ping.h"

extern t_ping g_ping;

void print_ping_header(const char* hostname,
                       const char* ip,
                       size_t payload_size) {
  uint16_t id;

  id = getpid() & 0xFFFF;
  if (g_ping.verbose)
    printf("PING %s (%s): %zu data bytes, id 0x%04x = %u\n", hostname, ip,
           payload_size, id, id);
  else
    printf("PING %s (%s): %zu data bytes\n", hostname, ip, payload_size);
}

void print_echo_reply(const struct iphdr* iphdr,
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

void print_ping_stats(const char* hostname) {
  double rtt_avg;
  double rtt_var;
  size_t packet_loss;

  printf("--- %s ping statistics ---\n", hostname);
  packet_loss = 0;
  if (g_ping.request_count > 0)
    packet_loss = (g_ping.request_count - g_ping.reply_count) * 100 /
                  g_ping.request_count;
  printf("%d packets transmitted, %d packets received, %zu%% packet loss\n",
         g_ping.request_count, g_ping.reply_count, packet_loss);
  if (packet_loss == 100)
    g_ping.exit_status = 1;
  if (g_ping.rtt_sum == 0 || g_ping.reply_count == 0)
    return;
  rtt_avg = g_ping.rtt_sum / g_ping.reply_count;
  rtt_var = sqrt(g_ping.rtt_sum_sq / g_ping.reply_count - rtt_avg * rtt_avg);
  if (rtt_var < 0)
    rtt_var = 0;
  printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
         g_ping.rtt_min, rtt_avg, g_ping.rtt_max, rtt_var);
}

void print_unknown_host_exit() {
  fprintf(stderr, "ping: unknown host\n");
  exit(1);
}

void print_checksum_mismatch(const struct iphdr* iphdr) {
  char replier_ip[INET_ADDRSTRLEN];

  inet_ntop(AF_INET, &iphdr->saddr, replier_ip, sizeof(replier_ip));
  printf("checksum mismatch from %s\n", replier_ip);
}

static const t_icmp_error g_icmp_errors[] = {
    {ICMP_DEST_UNREACH, ICMP_NET_UNREACH, "Destination Net Unreachable"},
    {ICMP_DEST_UNREACH, ICMP_HOST_UNREACH, "Destination Host Unreachable"},
    {ICMP_DEST_UNREACH, ICMP_PROT_UNREACH, "Destination Protocol Unreachable"},
    {ICMP_DEST_UNREACH, ICMP_PORT_UNREACH, "Destination Port Unreachable"},
    {ICMP_DEST_UNREACH, ICMP_FRAG_NEEDED, "Fragmentation needed and DF set"},
    {ICMP_DEST_UNREACH, ICMP_SR_FAILED, "Source Route Failed"},
    {ICMP_DEST_UNREACH, ICMP_NET_UNKNOWN, "Network Unknown"},
    {ICMP_DEST_UNREACH, ICMP_HOST_UNKNOWN, "Host Unknown"},
    {ICMP_DEST_UNREACH, ICMP_HOST_ISOLATED, "Host Isolated"},
    {ICMP_DEST_UNREACH, ICMP_NET_ANO, "Destination Network Prohibited"},
    {ICMP_DEST_UNREACH, ICMP_HOST_ANO, "Destination Host Prohibited"},
    {ICMP_DEST_UNREACH, ICMP_NET_UNR_TOS, "Network Unreachable for TOS"},
    {ICMP_DEST_UNREACH, ICMP_HOST_UNR_TOS, "Host Unreachable for TOS"},
    {ICMP_DEST_UNREACH, ICMP_PKT_FILTERED, "Packet filtered"},
    {ICMP_DEST_UNREACH, ICMP_PREC_VIOLATION, "Precedence Violation"},
    {ICMP_DEST_UNREACH, ICMP_PREC_CUTOFF, "Precedence Cutoff"},
    {ICMP_REDIRECT, ICMP_REDIR_NET, "Redirect Network"},
    {ICMP_REDIRECT, ICMP_REDIR_HOST, "Redirect Host"},
    {ICMP_REDIRECT, ICMP_REDIR_NETTOS, "Redirect Type of Service and Network"},
    {ICMP_REDIRECT, ICMP_REDIR_HOSTTOS, "Redirect Type of Service and Host"},
    {ICMP_TIME_EXCEEDED, ICMP_EXC_TTL, "Time to live exceeded"},
    {ICMP_TIME_EXCEEDED, ICMP_EXC_FRAGTIME, "Frag reassembly time exceeded"},
    {ICMP_SOURCE_QUENCH, 0, "Source Quench"},
    {ICMP_PARAMETERPROB, 0, "Parameter problem"},
};

static const char* icmp_error_msg(uint8_t type, uint8_t code) {
  for (size_t i = 0; i < sizeof(g_icmp_errors) / sizeof(*g_icmp_errors); i++)
    if (g_icmp_errors[i].type == type && g_icmp_errors[i].code == code)
      return g_icmp_errors[i].msg;

  return NULL;
}

void print_ip_dump(const struct iphdr* ip, const struct icmphdr* icmp) {
  const uint8_t* bytes = (const uint8_t*)ip;
  size_t hlen = ip->ihl * 4;
  uint16_t frag_off = ntohs(ip->frag_off);
  char src[INET_ADDRSTRLEN];
  char dst[INET_ADDRSTRLEN];

  printf("IP Hdr Dump:\n ");
  for (size_t i = 0; i < sizeof(struct iphdr); i++)
    printf("%02x%s", bytes[i], (i % 2) ? " " : "");
  printf("\n");

  inet_ntop(AF_INET, &ip->saddr, src, sizeof(src));
  inet_ntop(AF_INET, &ip->daddr, dst, sizeof(dst));
  printf("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src\tDst\tData\n");
  printf(" %1x  %1x  %02x %04x %04x   %1x %04x  %02x  %02x %04x %s  %s ",
         ip->version, ip->ihl, ip->tos, ntohs(ip->tot_len), ntohs(ip->id),
         (frag_off & 0xe000) >> 13, frag_off & 0x1fff, ip->ttl, ip->protocol,
         ntohs(ip->check), src, dst);
  for (size_t i = sizeof(struct iphdr); i < hlen; i++)  // IP options, if any
    printf("%02x", bytes[i]);
  printf("\n");

  printf("ICMP: type %u, code %u, size %u", icmp->type, icmp->code,
         ntohs(ip->tot_len) - (unsigned)hlen);
  if (icmp->type == ICMP_ECHO || icmp->type == ICMP_ECHOREPLY)
    printf(", id 0x%04x, seq 0x%04x", ntohs(icmp->un.echo.id),
           ntohs(icmp->un.echo.sequence));
  printf("\n");
}
