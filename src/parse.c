#include "ft_ping.h"

extern t_ping g_ping;

static void exit_64() {
  print_more_info();
  exit(64);
}

static int parse_payload_size(const char* arg) {
  char* stop;
  g_ping.payload_size = strtoul(arg, &stop, 0);

  if (*stop != '\0') {
    fprintf(stderr, "ping: invalid value (`%s' near `%s')\n", arg, stop);
    return -1;
  }
  if (g_ping.payload_size > PING_MAX_PAYLOAD_SIZE) {
    fprintf(stderr, "ping: option value too big: %s\n", arg);
    return -1;
  }
  return 0;
}

static int parse_packet_count(const char* arg) {
  char* stop;

  g_ping.count = strtoul(arg, &stop, 0);
  if (*stop != '\0') {
    fprintf(stderr, "ping: invalid value (`%s' near `%s')\n", arg, stop);
    return -1;
  }
  return 0;
}

static const char* get_long_option_name(const struct option* options, int val) {
  for (; options->name != NULL; options++)
    if (options->val == val)
      return options->name;
  return NULL;
}

void parse_arguments(int argc, char* argv[]) {
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
  int opt;
  int index = 0;
  opterr = 0;

  while ((opt = getopt_long(argc, argv, ":s:c:vV", options, &index)) != -1) {
    switch (opt) {
      case OPT_HELP:
        print_help();
        exit(0);
      case OPT_USAGE:
        print_usage();
        exit(0);
      case 'c':
        if (parse_packet_count(optarg) != 0)
          exit(1);
        break;
      case 's':
        if (parse_payload_size(optarg) != 0)
          exit(1);
        break;
      case 'v':
        g_ping.verbose = true;
        break;
      case 'V':
        print_version();
        exit(0);
      case ':':
        if (strncmp(argv[optind - 1], "--", 2) == 0)
          fprintf(stderr, "ping: option '--%s' requires an argument\n",
                  get_long_option_name(options, optopt));
        else
          fprintf(stderr, "ping: option requires an argument -- '%c'\n",
                  optopt);
        exit_64();
      case '?':
        if (optopt == '?') {
          print_help();
          exit(0);
        }
        if (optopt == 0)
          fprintf(stderr, "ping: unrecognized option '%s'\n", argv[optind - 1]);
        else
          fprintf(stderr, "ping: invalid option -- '%c'\n", optopt);
        exit_64();
      default:
        break;
    }
  }

  g_ping.host_count = argc - optind;
  if (g_ping.host_count <= 0) {
    fprintf(stderr, "ping: missing host operand\n");
    exit_64();
  }
  g_ping.hosts = argv + optind;
}