#include "ft_ping.h"

// RFC 1071 Internet checksum
uint16_t checksum(void* data, int len) {
  uint16_t* buf = data;
  uint32_t sum = 0;

  for (; len > 1; len -= 2)
    sum += *buf++;
  if (len == 1)
    sum += *(uint8_t*)buf;

  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  return (uint16_t)~sum;
}

long timeval_to_usec(struct timeval tv) {
  return tv.tv_sec * 1000000L + tv.tv_usec;
}

struct timeval usec_to_timeval(long us) {
  struct timeval tv = {.tv_sec = us / 1000000, .tv_usec = us % 1000000};
  return tv;
}