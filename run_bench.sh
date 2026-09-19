#!/bin/bash
set -x
cd "$(dirname "$0")"
DIR="$(pwd)"
WRK="$DIR/wrk"
URL="http://127.0.0.1:8080/sum?a=123&b=456"
OUT="$DIR/bench_results.txt"

pkill -f "$DIR/my_server" 2>/dev/null || true
sleep 0.5

run_one() {
  local threads=$1
  echo "========== SERVER THREADS: $threads ==========" >> "$OUT"
  "$DIR/my_server" "$threads" >> "$OUT" 2>&1 &
  local pid=$!
  sleep 1
  curl -s "http://127.0.0.1:8080/sum?a=1&b=2" >> "$OUT" 2>&1
  echo "" >> "$OUT"
  echo "--- wrk t1 c1 d10s ---" >> "$OUT"
  "$WRK" -t1 -c1 -d10s --latency "$URL" >> "$OUT" 2>&1
  echo "" >> "$OUT"
  echo "--- wrk t4 c100 d10s ---" >> "$OUT"
  "$WRK" -t4 -c100 -d10s --latency "$URL" >> "$OUT" 2>&1
  echo "" >> "$OUT"
  kill "$pid" 2>/dev/null
  wait "$pid" 2>/dev/null || true
  sleep 0.5
}

: > "$OUT"
run_one 1
run_one 4
echo DONE >> "$OUT"
