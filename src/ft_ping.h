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

#define PING_PAYLOAD_SIZE 56
#define PING_PACKET_SIZE (sizeof(struct icmphdr) + PING_PAYLOAD_SIZE)

typedef struct {
  int verbose;
} t_flags;

typedef struct {
  const char* hostname;
  char ip[INET_ADDRSTRLEN];
  struct sockaddr_in addr;
} t_target;

typedef struct {
  struct icmphdr hdr;
  char payload[PING_PAYLOAD_SIZE];
} t_icmp_packet;

extern t_flags g_flags;

void print_usage();
void print_help();
void print_version();
void print_more_info();

#endif