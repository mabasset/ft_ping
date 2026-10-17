#include "ft_ping.h"

extern t_ping g_ping;

static const t_icmp_error icmp_errors[] = {
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

static char* get_error_msg(uint8_t type, uint8_t code) {
  for (size_t i = 0; i < sizeof(icmp_errors) / sizeof(*icmp_errors); i++)
    if (icmp_errors[i].type == type && icmp_errors[i].code == code)
      return icmp_errors[i].msg;

  return NULL;
}

static void translate_saddr(uint32_t saddr, char* out, size_t out_size) {
  char hostname[NI_MAXHOST];
  char ip[INET_ADDRSTRLEN];
  struct sockaddr_in sa = {0};

  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = saddr;
  inet_ntop(AF_INET, &sa.sin_addr, ip, sizeof(ip));
  if (getnameinfo((struct sockaddr*)&sa, sizeof(sa), hostname, sizeof(hostname),
                  NULL, 0, NI_NAMEREQD) == 0)
    snprintf(out, out_size, "%s (%s)", hostname, ip);
  else
    snprintf(out, out_size, "%s", ip);
}

void print_reply_error(const struct iphdr* iphdr,
                       const t_icmp_packet* icmp_packet) {
  char from[64];
  char* msg;

  translate_saddr(iphdr->saddr, from, sizeof(from));
  msg = get_error_msg(icmp_packet->hdr.type, icmp_packet->hdr.code);
  if (msg == NULL)
    msg = "Unknown Error";
  printf("%zu bytes from %s: %s\n",
         sizeof(struct icmphdr) + icmp_packet->payload_len, from, msg);
}

void print_ip_dump(const struct iphdr* iphdr, const struct icmphdr* icmphdr) {
  const uint8_t* bytes;
  size_t hlen;
  uint16_t frag_off;
  char src[INET_ADDRSTRLEN];
  char dst[INET_ADDRSTRLEN];

  bytes = (const uint8_t*)iphdr;
  printf("IP Hdr Dump:\n ");
  for (size_t i = 0; i < sizeof(struct iphdr); i++)
    printf("%02x%s", bytes[i], (i % 2) ? " " : "");
  printf("\n");

  inet_ntop(AF_INET, &iphdr->saddr, src, sizeof(src));
  inet_ntop(AF_INET, &iphdr->daddr, dst, sizeof(dst));
  frag_off = ntohs(iphdr->frag_off);
  printf("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src\tDst\tData\n");
  printf(" %1x  %1x  %02x %04x %04x   %1x %04x  %02x  %02x %04x %s  %s ",
         iphdr->version, iphdr->ihl, iphdr->tos, ntohs(iphdr->tot_len),
         ntohs(iphdr->id), (frag_off & 0xe000) >> 13, frag_off & 0x1fff,
         iphdr->ttl, iphdr->protocol, ntohs(iphdr->check), src, dst);
  hlen = iphdr->ihl * 4;
  for (size_t i = sizeof(struct iphdr); i < hlen; i++)  // IP options, if any
    printf("%02x", bytes[i]);
  printf("\n");

  printf("ICMP: type %u, code %u, size %u", icmphdr->type, icmphdr->code,
         ntohs(iphdr->tot_len) - (unsigned)hlen);
  if (icmphdr->type == ICMP_ECHO || icmphdr->type == ICMP_ECHOREPLY)
    printf(", id 0x%04x, seq 0x%04x", ntohs(icmphdr->un.echo.id),
           ntohs(icmphdr->un.echo.sequence));
  printf("\n");
}
