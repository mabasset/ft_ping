#ifndef FT_PING_H
#define FT_PING_H

#include "../libmb/libmb.h"

#include <arpa/inet.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

#define PING_DEFAULT_PAYLOAD_SIZE 56
#define PING_MAX_PAYLOAD_SIZE 65399

typedef struct {
  const char* hostname;
  char ip[INET_ADDRSTRLEN];
  struct sockaddr_in addr;
} t_target;

typedef struct {
  struct icmphdr hdr;
  char payload[PING_MAX_PAYLOAD_SIZE];
} t_icmp_packet;

typedef struct {
  int sockfd;
  char** hosts;
  int host_count;
  volatile sig_atomic_t sigint;
  size_t request_count;
  size_t reply_count;

  bool verbose;
  size_t packet_limit;
  size_t payload_size;
  size_t linger_seconds;
} t_ping;

// print.c
void print_usage_exit();
void print_help_exit();
void print_version_exit();
void print_more_info_exit();
void print_invalid_value_exit(const char* stop);
void print_big_value_exit();
void print_ping_header(const char* hostname,
                       const char* ip,
                       const int payload_size);

// math.c
uint16_t checksum(void* data, int len);
long timeval_to_usec(struct timeval tv);
struct timeval usec_to_timeval(long us);

void parse_flags(int argc, char* argv[]);
void parse_hosts(int argc, char* argv[]);

void run_ping();

#endif