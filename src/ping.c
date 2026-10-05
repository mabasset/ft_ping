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
  if (g_ping.count && g_ping.request_count >= g_ping.count)
    return;
  gettimeofday(send_time, NULL);
  build_echo_request(packet, g_ping.request_count, send_time);
  sendto(g_ping.sockfd, packet, sizeof(packet->hdr) + g_ping.payload_size, 0,
         (struct sockaddr*)&addr, sizeof(addr));
  g_ping.request_count++;
}

static int receive_echo_reply() {
  char buffer[15 * 4 + sizeof(t_icmp_packet)];  // max IP header + ICMP packet
  struct sockaddr_in from;           // filled by the kernel: who sent it
  socklen_t fromlen = sizeof(from);  // in/out
  // struct timeval now;

  ssize_t n = recvfrom(g_ping.sockfd, buffer, sizeof(buffer), 0,
                       (struct sockaddr*)&from, &fromlen);
  printf("%d\n", n);
  // if (n < 0)
  //   return -1;
  // gettimeofday(&now, NULL);

  // struct iphdr* ip = (struct iphdr*)buffer;
  // size_t ip_len = ip->ihl * 4;
  // if ((size_t)n < ip_len + sizeof(struct icmphdr))
  //   return -1;  // truncated / garbage

  // struct icmphdr* icmp = (struct icmphdr*)(buffer + ip_len);
  // size_t icmp_len = n - ip_len;

  // // a raw socket sees ALL icmp traffic: keep only our reply
  // if (icmp->type != ICMP_ECHOREPLY ||
  //     icmp->un.echo.id != htons(getpid() & 0xFFFF))
  //   return 1;  // not ours: caller should keep waiting

  // char ip_str[INET_ADDRSTRLEN];
  // inet_ntop(AF_INET, &from.sin_addr, ip_str, sizeof(ip_str));

  // double rtt = (now.tv_sec - send_time.tv_sec) * 1000.0 +
  //              (now.tv_usec - send_time.tv_usec) / 1000.0;

  // printf("%zu bytes from %s: icmp_seq=%u ttl=%u time=%.3f ms\n", icmp_len,
  //        ip_str, seq, ip->ttl, rtt);
  // return 0;
}

static void set_select_timeout(struct timeval* select_timeout,
                               struct timeval send_time) {
  struct timeval timeout_limit = {.tv_sec = 1, .tv_usec = 0};
  struct timeval now;
  long usec;

  gettimeofday(&now, NULL);
  usec = timeval_to_usec(timeout_limit) -
         (timeval_to_usec(now) - timeval_to_usec(send_time));
  if (usec < 0)
    usec = 0;
  *select_timeout = usec_to_timeval(usec);
}

void run_ping() {
  t_target target;
  struct timeval send_time;
  struct timeval select_timeout;
  t_icmp_packet packet;
  fd_set fdset;
  int ready_fds;

  for (int i = 0; i < g_ping.host_count; i++) {
    resolve_target(g_ping.hosts[i], &target);
    print_ping_header(target.hostname, target.ip, g_ping.payload_size);
    send_echo_request(&packet, &send_time, target.addr);
    while (!g_ping.sigint &&
           (!g_ping.count || g_ping.reply_count < g_ping.count)) {
      FD_ZERO(&fdset);
      FD_SET(g_ping.sockfd, &fdset);
      set_select_timeout(&select_timeout, send_time);
      ready_fds =
          select(g_ping.sockfd + 1, &fdset, NULL, NULL, &select_timeout);
      if (ready_fds < 0) {
        if (errno == EINTR)
          continue;
        exit(1);
      } else if (ready_fds == 0)
        send_echo_request(&packet, &send_time, target.addr);
      else if (ready_fds == 1)
        receive_echo_reply();
    }

    printf("--- %s ping statistics ---\n", target.hostname);
    printf("%d packets transmitted, %d packets received, %d%% packet loss\n ",
           1, 1, 0);
    printf("round-trip min/avg/max/stddev = 19.075/21.289/30.544/3.335 ms\n");
  }
}