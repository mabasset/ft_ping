#ifndef FT_PING_H
#define FT_PING_H

#include "../libmb/libmb.h"

#include <arpa/inet.h>
#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#define PING_DEFAULT_PAYLOAD_SIZE 56
#define PING_MAX_PAYLOAD_SIZE 65399

typedef struct {
  bool verbose;
  size_t size;
} t_flags;

typedef struct {
  const char* hostname;
  char ip[INET_ADDRSTRLEN];
  struct sockaddr_in addr;
} t_target;

typedef struct {
  struct icmphdr hdr;
  char payload[PING_MAX_PAYLOAD_SIZE];
} t_icmp_packet;

// print.c
void print_usage();
void print_help();
void print_version();
void print_more_info();

// icmp.c
int resolve_target(const char* hostname, t_target* target);
void build_echo_request(t_icmp_packet* packet, uint16_t seq);

// math.c
uint16_t checksum(void* data, int len);

#endif