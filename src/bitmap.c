#include "ft_ping.h"

void bitmap_set(uint8_t* bitmap, uint16_t seq) {
  seq %= PING_SEQ_BITMAP_BITS;
  bitmap[seq / 8] |= (uint8_t)1 << (seq % 8);
}

void bitmap_clear(uint8_t* bitmap, uint16_t seq) {
  seq %= PING_SEQ_BITMAP_BITS;
  bitmap[seq / 8] &= ~((uint8_t)1 << (seq % 8));
}

int bitmap_test(const uint8_t* bitmap, uint16_t seq) {
  seq %= PING_SEQ_BITMAP_BITS;
  return (bitmap[seq / 8] >> (seq % 8)) & 1;
}