#include "ft_ping.h"

t_flags g_flags = {.size = PING_DEFAULT_PAYLOAD_SIZE, .verbose = false};

static int parse_size(const char* arg) {
  char* stop;
  g_flags.size = strtoul(arg, &stop, 0);

  if (*stop != '\0') {
    fprintf(stderr, "ping: invalid value (`%s' near `%s')\n", arg, stop);
    return -1;
  }
  if (g_flags.size > PING_MAX_PAYLOAD_SIZE) {
    fprintf(stderr, "ping: option value too big: %s\n", arg);
    return -1;
  }
  return 0;
}

static const char* long_option_name(const struct option* options, int val) {
  for (; options->name != NULL; options++)
    if (options->val == val)
      return options->name;
  return NULL;
}

int main(int argc, char* argv[]) {
  enum { OPT_HELP = 1, OPT_USAGE };
  static struct option options[] = {{"help", no_argument, NULL, OPT_HELP},
                                    {"usage", no_argument, NULL, OPT_USAGE},
                                    {"size", required_argument, NULL, 's'},
                                    {"verbose", no_argument, NULL, 'v'},
                                    {"version", no_argument, NULL, 'V'},
                                    {0, 0, 0, 0}};
  int opt;
  int index = 0;
  opterr = 0;

  // parse flags
  while ((opt = getopt_long(argc, argv, ":s:vV", options, &index)) != -1) {
    switch (opt) {
      case 0:
        break;
      case OPT_HELP:
        print_help();
        return 0;
      case OPT_USAGE:
        print_usage();
        return 0;
      case 's':
        if (parse_size(optarg) != 0)
          return 1;
        break;
      case 'v':
        g_flags.verbose = 1;
        break;
      case 'V':
        print_version();
        return 0;
      case ':':
        if (strncmp(argv[optind - 1], "--", 2) == 0)
          fprintf(stderr, "ping: option '--%s' requires an argument\n",
                  long_option_name(options, optopt));
        else
          fprintf(stderr, "ping: option requires an argument -- '%c'\n",
                  optopt);
        print_more_info();
        return 64;
      case '?':
        if (optopt == '?') {
          print_help();
          return 0;
        }
        if (optopt == 0)
          fprintf(stderr, "ping: unrecognized option '%s'\n", argv[optind - 1]);
        else
          fprintf(stderr, "ping: invalid option -- '%c'\n", optopt);
        print_more_info();
        return 64;
      default:
        break;
    }
  }

  if (optind >= argc) {
    fprintf(stderr, "ping: missing host operand\n");
    print_more_info();
    return 64;
  }

  /* open a socket
   * AF_INET: network layer ipv4
   * SOCK_RAW: no transport layer header
   * IPPROTO_ICMP: icmp messages
   */
  int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (sockfd == -1) {
    switch (errno) {
      case 1:
        fprintf(stderr, "ping: Lacking privilege for icmp socket.\n");
        break;
      default:
        fprintf(stderr, "ping: %s\n", strerror(errno));
        break;
    }
    return 1;
  }

  // drop sudo privileges
  if (setuid(getuid()) != 0) {
    fprintf(stderr, "ping: setuid: %s\n", strerror(errno));
    return 1;
  }

  // get the target infos
  t_target target;
  if (resolve_target(argv[optind], &target) != 0) {
    fprintf(stderr, "ping: unknown host\n");
    return 1;
  }

  printf("PING %s (%s): %ld data bytes\n", target.hostname, target.ip,
         g_flags.size);

  t_icmp_packet packet;
  build_echo_request(&packet, 0);

  if (sendto(sockfd, &packet, sizeof(packet.hdr) + g_flags.size, 0,
             (struct sockaddr*)&target.addr, sizeof(target.addr)) < 0) {
    fprintf(stderr, "ping: sendto: %s\n", strerror(errno));
    return 1;
  }

  return 0;
}