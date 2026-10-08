#!/bin/bash
# Compare ft_ping against the system ping (GNU inetutils).
#   ./tests/compare.sh               run every case below
#   ./tests/compare.sh -c3 1.1.1.1   run a single case
# Needs sudo for ft_ping (raw socket).

cd "$(dirname "$0")/.." || exit 1

normalize() {
  sed -E -e 's/time=[0-9.]+/time=X/' \
         -e 's/= [0-9.]+\/[0-9.]+\/[0-9.]+\/[0-9.]+ ms/= X\/X\/X\/X ms/' \
         -e 's/id 0x[0-9a-f]+ = [0-9]+/id X = X/'
}

run() {
  "$@" 2>&1 | normalize
  echo "exit=${PIPESTATUS[0]}"
}

compare() {
  local out
  if out=$(diff <(run ping "$@") <(run ./ft_ping "$@")); then
    echo "OK    $*"
  else
    echo "FAIL  $*"
    echo "$out" | sed 's/^/      /'
    return 1
  fi
}

if [ $# -gt 0 ]; then
  compare "$@"
  exit
fi

cases=(
  "-c3 127.0.0.1"
  "-c3 localhost"
  "-c3 -s 100 127.0.0.1"
  "-c3 -s 8 127.0.0.1"
  "-c2 -W2 192.0.2.1"
  "-v -c1 127.0.0.1"
  "-W 0 127.0.0.1"
  "-W -1 127.0.0.1"
  "-s 70000 127.0.0.1"
  "-c abc 127.0.0.1"
  "--linger 127.0.0.1"
  "-x 127.0.0.1"
  "--foo 127.0.0.1"
  "-c1 nonexistent.invalid"
  ""
)

fails=0
for c in "${cases[@]}"; do
  # shellcheck disable=SC2086
  compare $c || fails=$((fails + 1))
done
echo
echo "${#cases[@]} cases, $fails failed"
[ "$fails" -eq 0 ]
