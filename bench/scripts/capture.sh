#!/usr/bin/env bash
# bench/scripts/capture.sh
#
# Reproduces exactly what VS Code's "ESP-IDF: Monitor your device" button runs —
# it invokes idf_monitor.py directly, pointed at the already-built .elf, the same
# way the button's own task does. It does NOT go through `idf.py monitor`, so it
# never triggers a dependency check / reconfigure / rebuild. If nothing has been
# built and flashed yet for this target/config, build+flash it first with the
# normal VS Code buttons — this script only opens the monitor and captures it.
#
# Usage:
#   ./capture.sh <esp32|esp32c3> <base|aessw> [port]
#
# port is optional: if omitted, the script looks for exactly one serial device matching
# /dev/cu.usbserial-* (FTDI/CP210x-style bridge, typically ESP32 dev boards) or
# /dev/cu.wchusbserial* (CH340-style bridge, typically ESP32-C3 dev boards) and uses it.
# If it finds none or more than one, it lists what it found and asks you to pass the
# right one explicitly as the 3rd argument (check with `ls /dev/cu.*`).
#
# Output goes to results/logs/<target>_<config>.log (repo root, sibling of bench/),
# both streamed to your terminal (via tee) and saved for Step 4's CSV assembly.

set -euo pipefail

# --- Fixed to this machine's IDF v5.5.5 install, taken verbatim from the
#     VS Code monitor button's own invocation. Update these three lines if you
#     ever switch IDF versions/toolchain locations. ---
IDF_PATH_VAL="/Users/emad/.espressif/v5.5.5/esp-idf"
PYTHON_BIN="/Users/emad/.espressif/tools/python/v5.5.5/venv/bin/python"
export IDF_PATH="$IDF_PATH_VAL"
IDF_MONITOR="$IDF_PATH_VAL/tools/idf_monitor.py"
IDF_PY="$IDF_PATH_VAL/tools/idf.py"

usage() {
  echo "Usage: $0 <esp32|esp32c3> <base|aessw> [port]" >&2
  exit 1
}

[ $# -lt 2 ] && usage
TARGET="$1"
LABEL="$2"
PORT="${3:-}"

case "$TARGET" in
  esp32)   TOOLCHAIN_PREFIX="xtensa-esp32-elf-" ;;
  esp32c3) TOOLCHAIN_PREFIX="riscv32-esp-elf-" ;;
  *) echo "Unknown target '$TARGET' (expected esp32 or esp32c3)" >&2; exit 1 ;;
esac

case "$LABEL" in
  base|aessw) ;;
  *) echo "Unknown config '$LABEL' (expected base or aessw)" >&2; exit 1 ;;
esac

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BENCH_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
REPO_ROOT="$(cd "$BENCH_DIR/.." && pwd)"

# esp32/esp32c3 (base) build under build/<target>/, the aes-sw configs build
# under build/<target>-aes-sw/ per the CMakePresets.json in the Phase 3 guide.
if [ "$LABEL" = "aessw" ]; then
  ELF="$BENCH_DIR/build/${TARGET}-aes-sw/bench.elf"
else
  ELF="$BENCH_DIR/build/${TARGET}/bench.elf"
fi

LOG_DIR="$REPO_ROOT/results/logs"
mkdir -p "$LOG_DIR"
LOG_FILE="$LOG_DIR/${TARGET}_${LABEL}.log"

if [ ! -f "$ELF" ]; then
  echo "No build found at: $ELF" >&2
  echo "Build and flash this configuration first (normal VS Code buttons), then re-run this script to capture it." >&2
  exit 1
fi

if [ -z "$PORT" ]; then
  # ESP32 dev boards typically enumerate as /dev/cu.usbserial-*  (FTDI/CP210x-style bridge);
  # ESP32-C3 dev boards typically enumerate as /dev/cu.wchusbserial* (CH340-style bridge).
  ports=()
  for pattern in /dev/cu.usbserial-* /dev/cu.wchusbserial*; do
    for p in $pattern; do
      [ -e "$p" ] && ports+=("$p")
    done
  done
  if [ ${#ports[@]} -eq 0 ]; then
    echo "No /dev/cu.usbserial-* or /dev/cu.wchusbserial* device found. Is the board plugged in and powered?" >&2
    echo "If it enumerates under a different name, check with: ls /dev/cu.*  and pass it as the 3rd argument." >&2
    exit 1
  elif [ ${#ports[@]} -gt 1 ]; then
    echo "Multiple serial devices found — pass the right one explicitly as the 3rd argument:" >&2
    printf '  %s\n' "${ports[@]}" >&2
    exit 1
  fi
  PORT="${ports[0]}"
fi

echo "Target:     $TARGET"
echo "Config:     $LABEL"
echo "Port:       $PORT"
echo "ELF:        $ELF"
echo "Logging to: $LOG_FILE"
echo "Press Ctrl+] to stop once you see 'BENCH: matrix complete'."
echo

"$PYTHON_BIN" "$IDF_MONITOR" -p "$PORT" -b 115200 --toolchain-prefix "$TOOLCHAIN_PREFIX" \
  --make "'$PYTHON_BIN' '$IDF_PY'" --target "$TARGET" "$ELF" | tee "$LOG_FILE"