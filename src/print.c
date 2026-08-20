#include "ft_ping.h"

void print_usage() {
  char* usage =
      "Usage: ping [-dnrvfqR?V] [-t TYPE] [-c NUMBER] [-i NUMBER] [-T NUM] [-w "
      "N]\n"
      "\t[-W N] [-l NUMBER] [-p PATTERN] [-s NUMBER] [--address] [--echo]\n"
      "\t[--mask] [--timestamp] [--type=TYPE] [--count=NUMBER] [--debug]\n"
      "\t[--interval=NUMBER] [--numeric] [--ignore-routing] [--ttl=N]\n"
      "\t[--tos=NUM] [--verbose] [--timeout=N] [--linger=N] [--flood]\n"
      "\t[--ip-timestamp=FLAG] [--preload=NUMBER] [--pattern=PATTERN]\n"
      "\t[--quiet] [--route] [--size=NUMBER] [--help] [--usage] [--version]\n"
      "\tHOST ...\n";
  ft_putstr(usage);
}