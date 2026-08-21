#ifndef FT_NM_H
#define FT_NM_H

#include "../libmb/libmb.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  int verbose;
} t_flags;

void print_usage();
void print_help();

#endif