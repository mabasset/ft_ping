#include "ft_ping.h"

extern t_ping g_ping;

void print_usage_exit() {
  char* usage =
      "Usage: ping [-dnrvfqR?V] [-t TYPE] [-c NUMBER] [-i NUMBER] [-T NUM] [-w "
      "N]\n"
      "            [-W N] [-l NUMBER] [-p PATTERN] [-s NUMBER] [--address] "
      "[--echo]\n"
      "            [--mask] [--timestamp] [--type=TYPE] [--count=NUMBER] "
      "[--debug]\n"
      "            [--interval=NUMBER] [--numeric] [--ignore-routing] "
      "[--ttl=N]\n"
      "            [--tos=NUM] [--verbose] [--timeout=N] [--linger=N] "
      "[--flood]\n"
      "            [--ip-timestamp=FLAG] [--preload=NUMBER] "
      "[--pattern=PATTERN]\n"
      "            [--quiet] [--route] [--size=NUMBER] [--help] [--usage] "
      "[--version]\n"
      "            HOST ...\n";
  ft_putstr(usage);
  exit(0);
}

void print_help_exit() {
  char* help =
      "Usage: ping [OPTION...] HOST ...\n"
      "Send ICMP ECHO_REQUEST packets to network hosts.\n\n"
      " Options controlling ICMP request types:\n"
      "      --address              send ICMP_ADDRESS packets (root only)\n"
      "      --echo                 send ICMP_ECHO packets (default)\n"
      "      --mask                 same as --address\n"
      "      --timestamp            send ICMP_TIMESTAMP packets\n"
      "  -t, --type=TYPE            send TYPE packets\n\n"
      " Options valid for all request types:\n\n"
      "  -c, --count=NUMBER         stop after sending NUMBER packets\n"
      "  -d, --debug                set the SO_DEBUG option\n"
      "  -i, --interval=NUMBER      wait NUMBER seconds between sending each "
      "packet\n"
      "  -n, --numeric              do not resolve host addresses\n"
      "  -r, --ignore-routing       send directly to a host on an attached "
      "network\n"
      "      --ttl=N                specify N as time-to-live\n"
      "  -T, --tos=NUM              set type of service (TOS) to NUM\n"
      "  -v, --verbose              verbose output\n"
      "  -w, --timeout=N            stop after N seconds\n"
      "  -W, --linger=N             number of seconds to wait for response\n\n"
      " Options valid for --echo requests:\n\n"
      "  -f, --flood                flood ping (root only)\n"
      "      --ip-timestamp=FLAG    IP timestamp of type FLAG, which is one "
      "of\n"
      "                             \"tsonly\" and \"tsaddr\"\n"
      "  -l, --preload=NUMBER       send NUMBER packets as fast as possible "
      "before\n"
      "                             falling into normal mode of behavior (root "
      "only)\n"
      "  -p, --pattern=PATTERN      fill ICMP packet with given pattern (hex)\n"
      "  -q, --quiet                quiet output\n"
      "  -R, --route                record route\n"
      "  -s, --size=NUMBER          send NUMBER data octets\n\n"
      "  -?, --help                 give this help list\n"
      "      --usage                give a short usage message\n"
      "  -V, --version              print program version\n\n"
      "Mandatory or optional arguments to long options are also mandatory or "
      "optional\n"
      "for any corresponding short options.\n\n"
      "Options marked with (root only) are available only to superuser.\n\n"
      "Report bugs to <bug-inetutils@gnu.org>.\n";
  ft_putstr(help);
  exit(0);
}

void print_version_exit() {
  char* version =
      "ping (GNU inetutils) 2.0\n"
      "Copyright(C) 2021 Free Software Foundation, Inc.\n"
      "License GPLv3 + : GNU GPL version 3 or later "
      "<https://gnu.org/licenses/gpl.html>.\n"
      "This is free software: you are free to change and redistribute it.\n"
      "There is NO WARRANTY, to the extent permitted by law.\n\n"
      "Written by Sergey Poznyakoff.\n";
  ft_putstr(version);
  exit(0);
}

void print_more_info_exit() {
  fprintf(stderr,
          "Try 'ping --help' or 'ping --usage' for more information.\n");
  exit(64);
}

void print_invalid_value_exit(const char* stop) {
  fprintf(stderr, "ping: invalid value (`%s' near `%s')\n", optarg, stop);
  exit(1);
}

void print_big_value_exit() {
  fprintf(stderr, "ping: option value too big: %s\n", optarg);
  exit(1);
}

void print_ping_header(const char* hostname,
                       const char* ip,
                       const int payload_size) {
  printf("PING %s (%s): %ld data bytes\n", hostname, ip, payload_size);
}

double print_echo_reply(const struct iphdr* iphdr,
                        const t_icmp_packet* icmp_packet,
                        struct timeval recv_time) {
  uint16_t sequence;
  char replier_ip[INET_ADDRSTRLEN];
  struct timeval send_time;
  double rtt;

  sequence = ntohs(icmp_packet->hdr.un.echo.sequence);
  inet_ntop(AF_INET, &iphdr->saddr, replier_ip, sizeof(replier_ip));
  if (icmp_packet->payload_len < sizeof(struct timeval)) {
    printf("%zu bytes from %s: icmp_seq=%u ttl=%u\n",
           icmp_packet->payload_len + sizeof(struct icmphdr), replier_ip,
           sequence, iphdr->ttl);
    return 0;
  }
  memcpy(&send_time, icmp_packet->payload, sizeof(send_time));
  rtt = timeval_to_ms(recv_time) - timeval_to_ms(send_time);
  printf("%zu bytes from %s: icmp_seq=%u ttl=%u time=%.3f ms\n",
         icmp_packet->payload_len + sizeof(struct icmphdr), replier_ip,
         sequence, iphdr->ttl, rtt);

  return rtt;
}

void print_ping_stats(const char* hostname) {
  double rtt_avg;
  double rtt_var;

  printf("--- %s ping statistics ---\n", hostname);
  printf("%d packets transmitted, %d packets received, %d%% packet loss\n",
         g_ping.request_count, g_ping.reply_count, 0);
  if (g_ping.reply_count == 0)
    return;
  rtt_avg = g_ping.rtt_sum / g_ping.reply_count;
  rtt_var = sqrt(g_ping.rtt_sum_sq / g_ping.reply_count - rtt_avg * rtt_avg);
  if (rtt_var < 0)
    rtt_var = 0;
  printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
         g_ping.rtt_min, rtt_avg, g_ping.rtt_max, rtt_var);
}