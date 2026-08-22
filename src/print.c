#include "ft_ping.h"

void print_usage() {
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
}

void print_help() {
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
}

void print_version() {
  char* version =
      "ping (GNU inetutils) 2.0\n"
      "Copyright(C) 2021 Free Software Foundation, Inc.\n"
      "License GPLv3 + : GNU GPL version 3 or later "
      "<https://gnu.org/licenses/gpl.html>.\n"
      "This is free software: you are free to change and redistribute it.\n"
      "There is NO WARRANTY, to the extent permitted by law.\n\n"
      "Written by Sergey Poznyakoff.\n";
  ft_putstr(version);
}

void print_more_info() {
  fprintf(stderr,
          "Try 'ping --help' or 'ping --usage' for more information.\n");
}