#ifndef FT_PING_H
#define FT_PING_H

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
#define PING_MIN_PAYLOAD_SIZE 0
#define PING_MAX_PAYLOAD_SIZE 65399

#define PING_DEFAULT_LINGER_SEC 10
#define PING_MIN_LINGER_SEC 1
#define PING_MAX_LINGER_SEC INT_MAX

#define PING_MIN_INTERVAL_SEC 0.2
#define PING_MAX_INTERVAL_SEC INT_MAX

#define PING_MIN_TTL 0
#define PING_MAX_TTL 255

#define PING_MAX_IPHDR_SIZE 60

#define PING_SEQ_BITMAP_BITS 1024

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
  volatile sig_atomic_t sigint;
  int exit_status;

  bool request_limit_reached;
  size_t request_count;
  size_t reply_count;
  size_t error_count;
  size_t duplicate_count;
  uint8_t seq_bitmap[PING_SEQ_BITMAP_BITS / 8];
  double rtt_max;
  double rtt_min;
  double rtt_sum;
  double rtt_sum_sq;

  bool verbose;
  size_t packet_limit;
  size_t payload_size;
  double interval;
  size_t linger;
  size_t ttl;
} t_ping;

typedef struct {
  uint8_t type;
  uint8_t code;
  char* msg;
} t_icmp_error;

// parse_print.c
void print_usage_exit();
void print_help_exit();
void print_version_exit();
void print_more_info_exit();
void print_invalid_value_exit(const char* stop);
void print_big_value_exit();
void print_small_value_exit();
void print_missing_host_exit();

// math.c
uint16_t checksum(void* data, int len);
long long timeval_to_usec(struct timeval tv);
double timeval_to_ms(struct timeval tv);
struct timeval usec_to_timeval(long long us);

void parse_flags(int argc, char* argv[]);

void run_ping(char** hosts, int host_count);

void send_request(t_icmp_packet* packet,
                  struct timeval* send_time,
                  struct sockaddr_in addr);

void receive_reply();

void print_reply_error(const struct iphdr* iphdr,
                       const t_icmp_packet* icmp_packet);
void print_ip_dump(const struct iphdr* iphdr,
                   const struct icmphdr* icmp_packet);

void bitmap_set(uint8_t* bitmap, uint16_t seq);
void bitmap_clear(uint8_t* bitmap, uint16_t seq);
int bitmap_test(const uint8_t* bitmap, uint16_t seq);

#endif