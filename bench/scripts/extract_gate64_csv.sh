#!/usr/bin/env bash
set -euo pipefail
CSV_HEADER='board,algorithm,impl,operation,payload_bytes,ad_bytes,iteration,cycles'
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
DATE_TAG="$(date +%Y%m%d)"
for BOARD in esp32 esp32c3; do
  LOG="$REPO_ROOT/results/logs/${BOARD}_gate64.log"
  [ -f "$LOG" ] || { echo "[SKIP] $BOARD — no $LOG"; continue; }
  OUT="$REPO_ROOT/results/raw/gate64_${BOARD}_${DATE_TAG}.csv"
  awk -v hdr="$CSV_HEADER" -v board="$BOARD" '
    { gsub(/\r$/, "") }
    !seen_header { if ($0 == hdr) seen_header=1; next }
    index($0, board ",") == 1 { print }
  ' "$LOG" | { echo "$CSV_HEADER"; cat; } > "$OUT"
  echo "-> $OUT ($(($(wc -l < "$OUT") - 1)) data rows, expect 128)"
done