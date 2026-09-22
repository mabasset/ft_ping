#ifndef FT_PING_H
#define FT_PING_H

#include "../libmb/libmb.h"

#include <arpa/inet.h>
#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
  int verbose;
} t_flags;

typedef struct {
  const char* hostname;
  char ip[INET_ADDRSTRLEN];
  struct sockaddr_in addr;
} t_target;

extern t_flags g_flags;

void print_usage();
void print_help();
void print_version();
void print_more_info();

#endif