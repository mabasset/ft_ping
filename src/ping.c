#include "ft_ping.h"

extern t_ping g_ping;

static void reset_ping() {
  g_ping.packet_limit_reached = false;
  g_ping.request_count = 0;
  g_ping.reply_count = 0;

  g_ping.rtt_max = 0;
  g_ping.rtt_min = 0;
  g_ping.rtt_sum = 0;
  g_ping.rtt_sum_sq = 0;
}

static void resolve_target(const char* hostname, t_target* target) {
  struct addrinfo hints = {0};
  struct addrinfo* res;

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_RAW;
  hints.ai_protocol = IPPROTO_ICMP;
  hints.ai_flags = 0;
  if (getaddrinfo(hostname, NULL, &hints, &res) != 0)
    print_unknown_host_exit();
  target->hostname = hostname;
  target->addr = *(struct sockaddr_in*)res->ai_addr;
  inet_ntop(AF_INET, &target->addr.sin_addr, target->ip, sizeof(target->ip));
  freeaddrinfo(res);
}

static long estimate_select_timeout(struct timeval send_time) {
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

  return timeout;
}

static int run_select(struct timeval send_time) {
  fd_set fdset;
  long usec_timeout;
  struct timeval timeval_timeout;
  int ready_fds;

  FD_ZERO(&fdset);
  FD_SET(g_ping.sockfd, &fdset);
  usec_timeout = estimate_select_timeout(send_time);
  if (usec_timeout == 0 && g_ping.packet_limit_reached)
    return -1;
  timeval_timeout = usec_to_timeval(usec_timeout);
  ready_fds = select(g_ping.sockfd + 1, &fdset, NULL, NULL, &timeval_timeout);
  if (ready_fds < 0) {
    if (errno == EINTR)
      return -1;
    perror("ping: select");
    exit(1);
  }

  return ready_fds;
}

void run_ping() {
  t_target target;
  struct timeval send_time;
  t_icmp_packet packet;
  int ready_fds;

  for (int i = 0; i < g_ping.host_count; i++) {
    reset_ping();
    resolve_target(g_ping.hosts[i], &target);
    print_ping_header(target.hostname, target.ip, g_ping.payload_size);
    send_echo_request(&packet, &send_time, target.addr);
    while (!g_ping.sigint && (g_ping.packet_limit == 0 ||
                              g_ping.reply_count < g_ping.packet_limit)) {
      ready_fds = run_select(send_time);
      if (ready_fds < 0)
        break;
      else if (ready_fds == 0 && !g_ping.packet_limit_reached)
        send_echo_request(&packet, &send_time, target.addr);
      else if (ready_fds == 1)
        receive_echo_reply();
    }
    print_ping_stats(target.hostname);
  }
}