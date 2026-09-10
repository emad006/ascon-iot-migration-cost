#!/usr/bin/env python3
"""Phase 4 — Bachmann opt64 gate check.
Usage: python3 analyze_gate.py results/raw/gate64_esp32_<date>.csv results/raw/gate64_esp32c3_<date>.csv
"""
import csv, sys, statistics

TARGETS = {
    ("esp32",   "Ascon128a"): 85.01,
    ("esp32",   "Ascon128"):  127.49,
    ("esp32c3", "Ascon128a"): 70.83,
    ("esp32c3", "Ascon128"):  102.90,
}
MARGIN = 0.15  # pre-declared in BUILD_PLAN.md / docs/PHASE4_MARGIN_DECLARATION.md

def load_rows(paths):
    rows = []
    for p in paths:
        with open(p, newline="") as f:
            lines = f.read().splitlines()
            start = next(i for i, l in enumerate(lines) if l.startswith("board,algorithm"))
            rows.extend(csv.DictReader(lines[start:]))
    return rows

def main(paths):
    rows = load_rows(paths)
    groups = {}
    for r in rows:
        groups.setdefault((r["board"], r["algorithm"], r["operation"]), []).append(int(r["cycles"]))

    print(f"{'board':<10}{'algorithm':<12}{'enc c/B':>10}{'dec c/B':>10}{'avg c/B':>10}"
          f"{'target':>10}{'delta%':>9}  gate")
    all_pass = True
    for board in ("esp32", "esp32c3"):
        for algo in ("Ascon128a", "Ascon128"):
            enc, dec = groups.get((board, algo, "encrypt")), groups.get((board, algo, "decrypt"))
            if not enc or not dec:
                print(f"{board:<10}{algo:<12}  MISSING DATA"); all_pass = False; continue
            enc_cpb = statistics.median(enc) / 32768
            dec_cpb = statistics.median(dec) / 32768
            avg_cpb = (enc_cpb + dec_cpb) / 2
            target = TARGETS[(board, algo)]
            delta = (avg_cpb - target) / target * 100
            ok = abs(delta) <= MARGIN * 100
            all_pass &= ok
            print(f"{board:<10}{algo:<12}{enc_cpb:>10.2f}{dec_cpb:>10.2f}{avg_cpb:>10.2f}"
                  f"{target:>10.2f}{delta:>+8.1f}%  {'PASS' if ok else 'FAIL'}")

    print("\nPHASE 4 GATE:", "PASS -- proceed to Phase 5" if all_pass else "FAIL -- do not proceed")
    return 0 if all_pass else 1

if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
