#include "ft_ping.h"

t_flags g_flags = {0};

int resolve_target(const char* host, t_target* target) {
  struct addrinfo hints = {0};
  struct addrinfo* res;

  hints.ai_family = AF_INET;

  if (getaddrinfo(host, NULL, &hints, &res) != 0)
    return -1;

  target->hostname = host;
  target->addr = *(struct sockaddr_in*)res->ai_addr;
  inet_ntop(AF_INET, &target->addr.sin_addr, target->ip, sizeof(target->ip));

  freeaddrinfo(res);
  return 0;
}

int main(int argc, char* argv[]) {
  enum { OPT_HELP = 1, OPT_USAGE };
  static struct option options[] = {{"verbose", no_argument, NULL, 'v'},
                                    {"version", no_argument, NULL, 'V'},
                                    {"help", no_argument, NULL, OPT_HELP},
                                    {"usage", no_argument, NULL, OPT_USAGE},
                                    {0, 0, 0, 0}};
  int opt;
  int index = 0;
  opterr = 0;

  while ((opt = getopt_long(argc, argv, "vV", options, &index)) != -1) {
    switch (opt) {
      case 0:
        break;
      case OPT_HELP:
        print_help();
        return 0;
      case OPT_USAGE:
        print_usage();
        return 0;
      case 'v':
        g_flags.verbose = 1;
        break;
      case 'V':
        print_version();
        return 0;
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

  t_target target;
  if (resolve_target(argv[optind], &target) != 0) {
    fprintf(stderr, "ping: unknown host\n");
    return 1;
  }

  printf("PING %s (%s): 56 data bytes\n", target.hostname, target.ip);

  return 0;
}