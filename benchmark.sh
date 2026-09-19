#!/bin/bash
set -e
DIR="$(cd "$(dirname "$0")" && pwd)"
WRK="${WRK:-$DIR/wrk}"
URL="http://127.0.0.1:8080/sum?a=123&b=456"

if [ ! -x "$WRK" ]; then
  echo "wrk not found at $WRK — build or set WRK=/path/to/wrk"
  exit 1
fi

echo "=== wrk: $("$WRK" --version 2>&1 | head -1) ==="
echo "URL: $URL"
echo ""

echo "--- Low concurrency (t1 c1, 10s) ---"
"$WRK" -t1 -c1 -d10s --latency "$URL"

echo ""
echo "--- Throughput (t4 c100, 10s) ---"
"$WRK" -t4 -c100 -d10s --latency "$URL"
