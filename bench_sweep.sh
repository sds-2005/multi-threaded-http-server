#!/bin/bash
# Sweep server threads × wrk configs; write results to bench_sweep.txt
set -e
cd "$(dirname "$0")"
WRK="$PWD/wrk"
URL="http://127.0.0.1:8080/sum?a=123&b=456"
OUT="$PWD/bench_sweep.txt"

make -q my_server 2>/dev/null || make

: > "$OUT"
echo "Project 1 throughput sweep — $(date -Iseconds)" >> "$OUT"
echo "URL: $URL" >> "$OUT"
echo "" >> "$OUT"

pkill -f "$PWD/my_server" 2>/dev/null || true
sleep 0.3

for THREADS in 1 2 4 6 8; do
  "$PWD/my_server" "$THREADS" >> /dev/null 2>&1 &
  PID=$!
  sleep 0.5

  for WRK_ARGS in \
    "-t1 -c1" \
    "-t1 -c10" \
    "-t4 -c100" \
    "-t6 -c200" \
    "-t6 -c400" \
    "-t8 -c400" \
    "-t8 -c800"; do
    echo "server_threads=$THREADS wrk $WRK_ARGS -d10s" >> "$OUT"
    "$WRK" $WRK_ARGS -d10s "$URL" 2>&1 | grep -E "Requests/sec|Latency|50%|99%" >> "$OUT"
    echo "" >> "$OUT"
  done

  kill "$PID" 2>/dev/null || true
  wait "$PID" 2>/dev/null || true
  sleep 0.3
done

echo "DONE" >> "$OUT"
echo "Results written to $OUT"
