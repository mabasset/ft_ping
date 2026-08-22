#include "ft_ping.h"

t_flags g_flags = {0};

int main(int argc, char* argv[]) {
  enum { OPT_HELP = 1, OPT_USAGE };
  static struct option options[] = {{"verbose", no_argument, NULL, 'v'},
                                    {"help", no_argument, NULL, OPT_HELP},
                                    {"usage", no_argument, NULL, OPT_USAGE},
                                    {0, 0, 0, 0}};
  int opt;
  int index = 0;
  opterr = 0;

  while ((opt = getopt_long(argc, argv, "v", options, &index)) != -1) {
    switch (opt) {
      case 0:
        break;
      case 'v':
        g_flags.verbose = 1;
        break;
      case OPT_HELP:
        print_help();
        return 0;
      case OPT_USAGE:
        print_usage();
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
        fprintf(stderr,
                "Try 'ping --help' or 'ping --usage' for more information.\n");
        return 64;
      default:
        break;
    }
  }

  printf("verbose = %d\n", g_flags.verbose);

  for (int i = optind; i < argc; i++)
    printf("Extra argument: %s\n", argv[i]);
  return 0;
}