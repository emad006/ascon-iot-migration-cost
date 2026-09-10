#!/usr/bin/env bash
set -euo pipefail

ASCON_NEW=bench/vendor/ascon-new
ASCON_OLD=bench/vendor/ascon-old

VARIANTS=(
  "aead128_opt32 | $ASCON_NEW | asconaead128 | opt32"
  "aead128_opt64 | $ASCON_NEW | asconaead128 | opt64"
  "128a_opt32    | $ASCON_OLD | ascon128av12 | opt32"
  "128a_opt64    | $ASCON_OLD | ascon128av12 | opt64"
  "128_opt32     | $ASCON_OLD | ascon128v12  | opt32"
  "128_opt64     | $ASCON_OLD | ascon128v12  | opt64"
)

mkdir -p /tmp/kat_out
FAIL=0

for entry in "${VARIANTS[@]}"; do
  IFS='|' read -r name repo algo variant <<< "$entry"
  name=$(xargs <<< "$name"); repo=$(xargs <<< "$repo")
  algo=$(xargs <<< "$algo"); variant=$(xargs <<< "$variant")

  src_dir="$repo/crypto_aead/$algo/$variant"
  kat_file="$repo/crypto_aead/$algo/LWC_AEAD_KAT_128_128.txt"
  run_dir="/tmp/kat_out/$name"
  bin="/tmp/kat_out/genkat_$name"

  echo "== $name  ($src_dir) =="
  gcc -O2 -I"$src_dir" "$src_dir"/*.c -I"$repo/tests" "$repo/tests/genkat_aead.c" -o "$bin"

  mkdir -p "$run_dir"
  (cd "$run_dir" && "$bin")   # writes its own LWC_AEAD_KAT_128_128.txt into $run_dir
  out="$run_dir/LWC_AEAD_KAT_128_128.txt"

  n=$(grep -c '^Count' "$out")
  if diff -q "$out" "$kat_file" > /dev/null; then
    echo "  PASS — $n/1089 vectors, byte-identical to $kat_file"
  else
    echo "  FAIL — diff against $kat_file:"
    diff "$out" "$kat_file" | head -20
    FAIL=1
  fi
done
exit $FAIL