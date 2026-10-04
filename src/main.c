#include "ft_ping.h"

t_ping g_ping = {.sockfd = -1, .payload_size = PING_DEFAULT_PAYLOAD_SIZE};

int send_echo_request(t_icmp_packet* packet,
                      uint16_t seq,
                      struct timeval* send_time,
                      struct sockaddr_in addr) {
  build_echo_request(packet, seq);
  gettimeofday(send_time, NULL);
  sendto(g_ping.sockfd, packet, sizeof(packet->hdr) + g_ping.payload_size, 0,
         (struct sockaddr*)&addr, sizeof(addr));
  return 0;
}

int receive_echo_reply(uint16_t seq, struct timeval send_time) {
  char buffer[15 * 4 + sizeof(t_icmp_packet)];  // max IP header + ICMP packet
  struct sockaddr_in from;           // filled by the kernel: who sent it
  socklen_t fromlen = sizeof(from);  // in/out
  struct timeval now;

  ssize_t n = recvfrom(g_ping.sockfd, buffer, sizeof(buffer), 0,
                       (struct sockaddr*)&from, &fromlen);
  if (n < 0)
    return -1;
  gettimeofday(&now, NULL);

  struct iphdr* ip = (struct iphdr*)buffer;
  size_t ip_len = ip->ihl * 4;
  if ((size_t)n < ip_len + sizeof(struct icmphdr))
    return -1;  // truncated / garbage

  struct icmphdr* icmp = (struct icmphdr*)(buffer + ip_len);
  size_t icmp_len = n - ip_len;

  // a raw socket sees ALL icmp traffic: keep only our reply
  if (icmp->type != ICMP_ECHOREPLY ||
      icmp->un.echo.id != htons(getpid() & 0xFFFF) ||
      ntohs(icmp->un.echo.sequence) != seq)
    return 1;  // not ours: caller should keep waiting

  char ip_str[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &from.sin_addr, ip_str, sizeof(ip_str));

  double rtt = (now.tv_sec - send_time.tv_sec) * 1000.0 +
               (now.tv_usec - send_time.tv_usec) / 1000.0;

  printf("%zu bytes from %s: icmp_seq=%u ttl=%u time=%.3f ms\n", icmp_len,
         ip_str, seq, ip->ttl, rtt);
  return 0;
}

static void cleanup(void) {
  printf("cioa\n");
  if (g_ping.sockfd >= 0)
    close(g_ping.sockfd);
}

static void create_socket() {
  g_ping.sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (g_ping.sockfd == -1)
    if (errno == 1)
      fprintf(stderr, "ping: Lacking privilege for icmp socket.\n");
    else
      perror("ping: socket");
  exit(1);
}

static void drop_sudo() {
  if (setuid(getuid()) != 0) {
    perror("ping: setuid");
    exit(1);
  }
}

int main(int argc, char* argv[]) {
  parse_arguments(argc, argv);
  create_socket();
  atexit(cleanup);
  drop_sudo();

  // t_target target;
  // for (int i = 0; i < g_ping.host_count; i++) {
  //   if (resolve_target(g_ping.hosts[i], &target) != 0) {
  //     fprintf(stderr, "ping: unknown host\n");
  //     return 1;
  //   }
  //   printf("PING %s (%s): %ld data bytes\n", target.hostname, target.ip,
  //          g_ping.payload_size);
  //   struct timeval send_time;
  //   struct timeval now;
  //   struct timeval time_left;
  //   struct timeval timeout = {.tv_sec = 1, .tv_usec = 0};
  //   t_icmp_packet packet;
  //   fd_set fdset;
  //   long usec_left;
  //   int ready_fds;

  //   send_echo_request(&packet, 0, &send_time, target.addr);

  //   while (1) {
  //     FD_ZERO(&fdset);
  //     FD_SET(g_ping.sockfd, &fdset);

  //     gettimeofday(&now, NULL);
  //     usec_left = timeval_to_usec(timeout) -
  //                 (timeval_to_usec(now) - timeval_to_usec(send_time));
  //     time_left = usec_to_timeval(usec_left);

  //     ready_fds = select(g_ping.sockfd + 1, &fdset, NULL, NULL, &time_left);
  //     if (ready_fds < 0)
  //       return 1;
  //     else if (ready_fds == 0)
  //       send_echo_request(&packet, 0, &send_time, target.addr);
  //     else if (ready_fds == 1)
  //       receive_echo_reply(0, send_time);
  //   }

  //   printf("--- %s ping statistics ---\n", target.hostname);
  //   printf("%d packets transmitted, %d packets received, %d%% packet loss\n",
  //   1,
  //          1, 0);
  //   printf("round-trip min/avg/max/stddev = 19.075/21.289/30.544/3.335
  //   ms\n");
  // }

  return 0;
}