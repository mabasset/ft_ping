#ifndef FT_PING_H
#define FT_PING_H

#include "../libmb/libmb.h"

#include <arpa/inet.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
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
#define PING_MAX_PAYLOAD_SIZE \
  65399  // 65535 - 60 (max IP hdr) - 76 (max ICMP error: 8 + 60 inner IP hdr +
         // 8)
#define PING_DEFAULT_LINGER_SEC 10
#define MAX_IPHDR_SIZE 60  // 15 * 4

typedef struct {
  const char* hostname;
  char ip[INET_ADDRSTRLEN];
  struct sockaddr_in addr;
} t_target;

typedef struct {
  struct icmphdr hdr;
  char payload[PING_MAX_PAYLOAD_SIZE];
  size_t payload_len;
} t_icmp_packet;

typedef struct {
  int sockfd;
  char** hosts;
  int host_count;
  volatile sig_atomic_t sigint;
  int exit_status;

  bool packet_limit_reached;
  size_t request_count;
  size_t reply_count;
  double rtt_max;
  double rtt_min;
  double rtt_sum;
  double rtt_sum_sq;

  bool verbose;
  size_t packet_limit;
  size_t payload_size;
  size_t linger_sec;
} t_ping;

typedef struct {
  uint8_t type;
  uint8_t code;
  const char* msg;
} t_icmp_error;

// parse_print.c
void print_usage_exit();
void print_help_exit();
void print_version_exit();
void print_more_info_exit();
void print_invalid_value_exit(const char* stop);
void print_big_value_exit();
void print_small_value_exit();

// ping_print.c
void print_ping_header(const char* hostname,
                       const char* ip,
                       size_t payload_size);
void print_echo_reply(const struct iphdr* iphdr,
                      const t_icmp_packet* icmp_packet,
                      double rtt);
void print_ping_stats(const char* hostname);
void print_unknown_host_exit();
void print_checksum_mismatch(const struct iphdr* iphdr);

// math.c
uint16_t checksum(void* data, int len);
long timeval_to_usec(struct timeval tv);
double timeval_to_ms(struct timeval tv);
struct timeval usec_to_timeval(long us);

void parse_flags(int argc, char* argv[]);
void parse_hosts(int argc, char* argv[]);

void run_ping();

void send_echo_request(t_icmp_packet* packet,
                       struct timeval* send_time,
                       struct sockaddr_in addr);
void receive_echo_reply();

#endif