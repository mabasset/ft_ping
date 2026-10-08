#include "ft_ping.h"

extern t_ping g_ping;

static void resolve_target(const char* hostname, t_target* target) {
  struct addrinfo hints = {0};
  struct addrinfo* res;

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_RAW;
  hints.ai_protocol = IPPROTO_ICMP;
  hints.ai_flags = 0;
  if (getaddrinfo(hostname, NULL, &hints, &res) != 0) {
    fprintf(stderr, "ping: unknown host\n");
    exit(1);
  }
  target->hostname = hostname;
  target->addr = *(struct sockaddr_in*)res->ai_addr;
  inet_ntop(AF_INET, &target->addr.sin_addr, target->ip, sizeof(target->ip));
  freeaddrinfo(res);
}

void build_echo_request(t_icmp_packet* packet,
                        uint16_t sequence,
                        struct timeval* send_time) {
  memset(packet, 0, sizeof(*packet));
  packet->hdr.type = ICMP_ECHO;
  packet->hdr.code = 0;
  packet->hdr.un.echo.id = htons(getpid() & 0xFFFF);
  packet->hdr.un.echo.sequence = htons(sequence);
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

static void send_echo_request(t_icmp_packet* packet,
                              struct timeval* send_time,
                              struct sockaddr_in addr) {
  gettimeofday(send_time, NULL);
  build_echo_request(packet, g_ping.request_count, send_time);
  sendto(g_ping.sockfd, packet, sizeof(packet->hdr) + g_ping.payload_size, 0,
         (struct sockaddr*)&addr, sizeof(addr));
  g_ping.request_count++;
  if (g_ping.packet_limit == g_ping.request_count)
    g_ping.packet_limit_reached = true;
}

static ssize_t read_packet(struct iphdr* iphdr,
                           t_icmp_packet* icmp_packet,
                           struct timeval* recv_time) {
  char buffer[15 * 4 + sizeof(struct icmphdr) +
              PING_MAX_PAYLOAD_SIZE];  // max IP header + ICMP packet
  ssize_t n_bytes;
  size_t iphdr_len;

  n_bytes = recv(g_ping.sockfd, buffer, sizeof(buffer), 0);
  gettimeofday(recv_time, NULL);
  if (n_bytes < 0 || (size_t)n_bytes < sizeof(struct iphdr))
    return -1;
  *iphdr = *(struct iphdr*)buffer;
  iphdr_len = iphdr->ihl * 4;
  if ((size_t)n_bytes < iphdr_len + sizeof(struct icmphdr))
    return -1;
  icmp_packet->hdr = *(struct icmphdr*)(buffer + iphdr_len);
  icmp_packet->payload_len = n_bytes - iphdr_len - sizeof(struct icmphdr);
  if (icmp_packet->payload_len > PING_MAX_PAYLOAD_SIZE)
    icmp_packet->payload_len = PING_MAX_PAYLOAD_SIZE;
  memcpy(icmp_packet->payload, buffer + iphdr_len + sizeof(struct icmphdr),
         icmp_packet->payload_len);

  return 0;
}

static void update_rtt_stats(double rtt) {
  if (rtt > g_ping.rtt_max)
    g_ping.rtt_max = rtt;
  if (rtt < g_ping.rtt_min || g_ping.rtt_min == 0)
    g_ping.rtt_min = rtt;
  g_ping.rtt_sum += rtt;
  g_ping.rtt_sum_sq += rtt * rtt;
}

static int receive_echo_reply() {
  struct iphdr iphdr;
  t_icmp_packet icmp_packet;
  struct timeval recv_time;
  double rtt;

  if (read_packet(&iphdr, &icmp_packet, &recv_time) != 0)
    return -1;
  if (icmp_packet.hdr.type != ICMP_ECHOREPLY ||
      icmp_packet.hdr.un.echo.id != htons(getpid() & 0xFFFF))
    return -1;
  g_ping.reply_count++;
  rtt = print_echo_reply(&iphdr, &icmp_packet, recv_time);
  if (rtt != 0)
    update_rtt_stats(rtt);

  return 0;
}

static int set_select_timeout(struct timeval* select_timeout,
                              struct timeval send_time) {
  struct timeval now;
  long timeout_limit;
  long timeout;

  timeout_limit = 1000000L;
  if (g_ping.packet_limit_reached)
    timeout_limit += g_ping.linger_sec * 1000000L;
  gettimeofday(&now, NULL);
  timeout = timeout_limit - (timeval_to_usec(now) - timeval_to_usec(send_time));
  if (timeout < 0)
    timeout = 0;
  if (timeout == 0 && g_ping.packet_limit_reached)
    return 1;
  *select_timeout = usec_to_timeval(timeout);

  return 0;
}

void reset_ping() {
  g_ping.packet_limit_reached = false;
  g_ping.request_count = 0;
  g_ping.reply_count = 0;

  g_ping.rtt_max = 0;
  g_ping.rtt_min = 0;
  g_ping.rtt_sum = 0;
  g_ping.rtt_sum_sq = 0;
}

void run_ping() {
  t_target target;
  struct timeval send_time;
  struct timeval select_timeout;
  t_icmp_packet packet;
  fd_set fdset;
  int ready_fds;

  for (int i = 0; i < g_ping.host_count; i++) {
    reset_ping();
    resolve_target(g_ping.hosts[i], &target);
    print_ping_header(target.hostname, target.ip, g_ping.payload_size);
    send_echo_request(&packet, &send_time, target.addr);
    while (!g_ping.sigint && (g_ping.packet_limit == 0 ||
                              g_ping.reply_count < g_ping.packet_limit)) {
      FD_ZERO(&fdset);
      FD_SET(g_ping.sockfd, &fdset);
      if (set_select_timeout(&select_timeout, send_time) != 0)
        break;
      ready_fds =
          select(g_ping.sockfd + 1, &fdset, NULL, NULL, &select_timeout);
      if (ready_fds < 0) {
        if (errno == EINTR)
          continue;
        exit(1);
      } else if (ready_fds == 0 && !g_ping.packet_limit_reached)
        send_echo_request(&packet, &send_time, target.addr);
      else if (ready_fds == 1)
        receive_echo_reply();
    }
    print_ping_stats(target.hostname);
  }
}