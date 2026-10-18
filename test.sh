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
  "-vc3 192.168.1.250"
)

prog="$1"
total=$(( ${#test_cases[@]} + 2))

for i in "${!test_cases[@]}"; do
  args="${test_cases[$i]}"

  echo "=== TEST $((i + 1))/${total}: $prog $args ==="
  timeout -s INT "$TIMEOUT" $prog $args 2>&1
  echo "--- $prog exit: $?"
  echo
  sleep "$SLEEP"
done

args="-vc10 localhost"

echo "=== TEST $(( ${total} - 1))/${total}: $prog $args ==="
sudo tc qdisc replace dev lo root netem corrupt 30%
$prog $args
echo "--- $prog exit: $?"
echo
sudo tc qdisc del dev lo root 2>/dev/null

sleep "$SLEEP"

echo "=== TEST ${total}/${total}: $prog $args ==="
sudo tc qdisc replace dev lo root netem duplicate 50%
$prog $args
echo "--- $prog exit: $?"
echo
sudo tc qdisc del dev lo root 2>/dev/null

echo "=== ALL TESTS DONE ==="
