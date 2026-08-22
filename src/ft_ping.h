#ifndef FT_PING_H
#define FT_PING_H

#include "../libmb/libmb.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int verbose;
} t_flags;

void print_usage();
void print_help();
void print_version();
void print_more_info();

#endif