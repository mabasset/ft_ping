#include "ft_ping.h"

extern t_ping g_ping;

enum { OPT_HELP = 1, OPT_USAGE };
static struct option options[] = {{"help", no_argument, NULL, OPT_HELP},
                                  {"usage", no_argument, NULL, OPT_USAGE},
                                  {"count", required_argument, NULL, 'c'},
                                  {"size", required_argument, NULL, 's'},
                                  {"verbose", no_argument, NULL, 'v'},
                                  {"version", no_argument, NULL, 'V'},
                                  {"timeout", required_argument, NULL, 'w'},
                                  {"linger", required_argument, NULL, 'W'},
                                  {0, 0, 0, 0}};

static size_t parse_optarg() {
  size_t num;
  char* stop;

  num = strtoul(optarg, &stop, 0);
  if (*stop != '\0')
    print_invalid_value_exit(stop);
  return num;
}

static size_t parse_optarg_limit(size_t min, size_t max) {
  size_t num;

  num = parse_optarg();
  if (num > max)
    print_big_value_exit();
  if (num < min)
    print_small_value_exit();
  return num;
}

static const char* get_long_option_name() {
  for (int i = 0; options[i].name != NULL; i++)
    if (options[i].val == optopt)
      return options[i].name;
  return NULL;
}

static void handle_missing_argument(char* argv[]) {
  if (strncmp(argv[optind - 1], "--", 2) == 0)
    fprintf(stderr, "ping: option '--%s' requires an argument\n",
            get_long_option_name());
  else
    fprintf(stderr, "ping: option requires an argument -- '%c'\n", optopt);
  print_more_info_exit();
}

static void handle_invalid_option(char* argv[]) {
  if (optopt == '?') {
    print_help_exit();
  }
  if (optopt == 0)
    fprintf(stderr, "ping: unrecognized option '%s'\n", argv[optind - 1]);
  else
    fprintf(stderr, "ping: invalid option -- '%c'\n", optopt);
  print_more_info_exit();
}

void parse_flags(int argc, char* argv[]) {
  int opt;
  int index = 0;
  opterr = 0;

  while ((opt = getopt_long(argc, argv, ":c:s:w:W:vV", options, &index)) !=
         -1) {
    switch (opt) {
      case OPT_HELP:
        print_help_exit();
      case OPT_USAGE:
        print_usage_exit();
      case 'c':
        g_ping.packet_limit = parse_optarg();
        break;
      case 's':
        g_ping.payload_size = parse_optarg_limit(0, PING_MAX_PAYLOAD_SIZE);
        break;
      case 'v':
        g_ping.verbose = true;
        break;
      case 'V':
        print_version_exit();
      case 'W':
        g_ping.linger_sec = parse_optarg_limit(1, INT_MAX);
        break;
      case ':':
        handle_missing_argument(argv);
      case '?':
        handle_invalid_option(argv);
      default:
        break;
    }
  }
}

void parse_hosts(int argc, char* argv[]) {
  g_ping.host_count = argc - optind;
  if (g_ping.host_count <= 0) {
    fprintf(stderr, "ping: missing host operand\n");
    print_more_info_exit();
  }
  g_ping.hosts = argv + optind;
}