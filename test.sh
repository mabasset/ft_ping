#!/bin/bash

TIMEOUT=10
SLEEP=3

test_cases=(
  "-c3 -s 100 127.0.0.1"
  "--count 010 -s 8 localhost"
  "-c1 192.0.2.1"
  "-c1 -W0x2 192.0.2.1"
  "-W 0 127.0.0.1"
  "-W -1 127.0.0.1"
  "-s 70000 127.0.0.1"
  "-c abc 127.0.0.1"
  "--linger 127.0.0.1"
  "-x 127.0.0.1"
  "--foo 127.0.0.1"
  "nonexistent.invalid"
  "--ttl 3 -c3 -v 192.178.194.100"
  "--interval 0 -c 3 localhost"
  "-i2 -c3 192.178.194.100"
)

prog="$1"

for i in "${!test_cases[@]}"; do
  args="${test_cases[$i]}"
  echo "=== TEST $((i + 1))/${#test_cases[@]}: $prog $args ==="
  timeout -s INT "$TIMEOUT" $prog $args 2>&1
  echo "--- $prog exit: $?"
  echo
  sleep "$SLEEP"
done
echo "=== ALL TESTS DONE ==="
