#include "ft_ping.h"

t_flags g_flags = {0};

int main(int argc, char* argv[]) {
  static struct option options[] = {{"verbose", no_argument, NULL, 'v'},
                                    {"help", no_argument, NULL, 'h'},
                                    {"usage", no_argument, NULL, 'u'},
                                    {0, 0, 0, 0}};
  int opt;
  int index = 0;

  while ((opt = getopt_long(argc, argv, "vhu", options, &index)) != -1) {
    switch (opt) {
      case 0:
        break;
      case 'v':
        g_flags.verbose = 1;
        break;
      case 'h':
        printf("Usage: %s [-v] [-o file] [--verbose] [--output file]\n",
               argv[0]);
        return 0;
      case 'u':
        print_usage();
        return 0;
      case '?':
        return 1;
      default:
        break;
    }
  }

  printf("verbose = %d\n", g_flags.verbose);

  for (int i = optind; i < argc; i++)
    printf("Extra argument: %s\n", argv[i]);
  return 0;
}