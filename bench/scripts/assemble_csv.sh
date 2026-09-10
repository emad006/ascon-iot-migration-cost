#!/usr/bin/env bash
# bench/scripts/assemble_csv.sh
#
# Assembles the per-board CSVs from results/logs/*.log (written by capture.sh)
# into results/raw/, and immediately checks each board's row count against the
# Step 3.6 gate (73,000 data rows) so a truncated capture is caught right away
# instead of silently carried into Phase 4.
#
# Usage:
#   ./assemble_csv.sh
#
# Expects results/logs/esp32_base.log, esp32_aessw.log, esp32c3_base.log,
# esp32c3_aessw.log (whichever exist — a board with a missing log is skipped
# and reported, not silently dropped).
#
# Every capture leaves a few bytes from the END of the *previous* session
# sitting in the USB-serial buffer, which get flushed out at the START of
# the next capture.log, before the real reset/boot banner even begins. That
# leftover fragment occasionally happens to start with a valid-looking
# "esp32," / "esp32c3," prefix, which would fool a plain `grep "^board,"`
# into miscounting it as a real data row. Since the app can never print a
# CSV data row before it prints the CSV header, extract_data() below simply
# ignores everything in a log until it has seen the real header line once,
# and only starts collecting matching rows after that point — structurally
# immune to this, no matter what garbage precedes the header.

set -euo pipefail

CSV_HEADER='board,algorithm,impl,operation,payload_bytes,ad_bytes,iteration,cycles'

# extract_data <board> <logfile>: prints only the data rows for <board> that
# appear strictly after the real CSV header line in <logfile>.
extract_data() {
  local board="$1" log="$2"
  awk -v board="$board" -v hdr="$CSV_HEADER" '
    { gsub(/\r$/, "") }   # logs are CRLF — strip trailing \r before comparing/printing
    !seen_header { if ($0 == hdr) seen_header=1; next }
    index($0, board ",") == 1 { print }
  ' "$log"
}

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BENCH_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
REPO_ROOT="$(cd "$BENCH_DIR/.." && pwd)"
LOG_DIR="$REPO_ROOT/results/logs"
RAW_DIR="$REPO_ROOT/results/raw"
mkdir -p "$RAW_DIR"
DATE_TAG="$(date +%Y%m%d)"
EXPECTED_ROWS=73000

if [ ! -d "$LOG_DIR" ]; then
  echo "No $LOG_DIR directory yet — run capture.sh first." >&2
  exit 1
fi

overall_status=0

for BOARD in esp32 esp32c3; do
  BASE_LOG="$LOG_DIR/${BOARD}_base.log"
  AESSW_LOG="$LOG_DIR/${BOARD}_aessw.log"

  if [ ! -f "$BASE_LOG" ] || [ ! -f "$AESSW_LOG" ]; then
    echo "[SKIP] $BOARD — missing log(s):"
    [ ! -f "$BASE_LOG" ]  && echo "         $BASE_LOG"
    [ ! -f "$AESSW_LOG" ] && echo "         $AESSW_LOG"
    continue
  fi

  OUT_CSV="$RAW_DIR/${BOARD}_${DATE_TAG}.csv"
  {
    echo "$CSV_HEADER"
    extract_data "$BOARD" "$BASE_LOG"
    extract_data "$BOARD" "$AESSW_LOG"
  } > "$OUT_CSV"

  ROWS=$(($(wc -l < "$OUT_CSV") - 1))
  if [ "$ROWS" -eq "$EXPECTED_ROWS" ]; then
    echo "[PASS] $BOARD -> $OUT_CSV ($ROWS data rows, expected $EXPECTED_ROWS)"
  else
    echo "[FAIL] $BOARD -> $OUT_CSV ($ROWS data rows, expected $EXPECTED_ROWS) — do NOT proceed to Phase 4 for this board until this is fixed"
    overall_status=1
  fi
done

STACK_CSV="$RAW_DIR/stack_${DATE_TAG}.csv"
{
  echo "stack,board,algorithm,impl,operation,payload_bytes,stack_free_bytes"
  for f in "$LOG_DIR"/*.log; do
    [ -f "$f" ] && extract_data "stack" "$f"
  done
} > "$STACK_CSV" || true
echo "Stack high-water-mark rows combined -> $STACK_CSV"

exit $overall_status