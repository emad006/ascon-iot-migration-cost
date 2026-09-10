#!/usr/bin/env python3
"""Convert a NIST-LWC AEAD KAT file (Count/Key/Nonce/PT/AD/CT) into a C header
of packed binary vectors for firmware embedding.
Usage: kat_to_c.py <input.txt> <c_identifier_prefix> > <prefix>_vectors.h
"""
import sys

def parse_kat(path):
    vectors, cur = [], {}
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line:
                if cur: vectors.append(cur); cur = {}
                continue
            if "=" not in line: continue
            k, _, v = line.partition("=")
            cur[k.strip()] = v.strip()
        if cur: vectors.append(cur)
    return vectors

def carr(h):
    return ", ".join(f"0x{h[i:i+2]}" for i in range(0, len(h), 2)) if h else ""

def main():
    path, prefix = sys.argv[1], sys.argv[2]
    vecs = parse_kat(path)
    print(f"/* auto-generated from {path}: {len(vecs)} vectors — do not hand-edit */")
    print("#include <stddef.h>\n#include <stdint.h>\n")
    print(f"typedef struct {{ int count; uint8_t key[16]; uint8_t nonce[16];"
          f" const uint8_t *pt; size_t pt_len; const uint8_t *ad; size_t ad_len;"
          f" const uint8_t *ct; size_t ct_len; }} {prefix}_vec_t;\n")
    for i, v in enumerate(vecs):
        for field in ("PT", "AD", "CT"):
            print(f"static const uint8_t {prefix}_{i}_{field.lower()}[] = {{{carr(v.get(field,''))}}};")
    print(f"\nstatic const {prefix}_vec_t {prefix}_vectors[] = {{")
    for i, v in enumerate(vecs):
        print(f"  {{ {v.get('Count','0')}, {{{carr(v.get('Key',''))}}}, {{{carr(v.get('Nonce',''))}}}, "
              f"{prefix}_{i}_pt, {len(v.get('PT',''))//2}, "
              f"{prefix}_{i}_ad, {len(v.get('AD',''))//2}, "
              f"{prefix}_{i}_ct, {len(v.get('CT',''))//2} }},")
    print("};")
    print(f"#define {prefix.upper()}_NUM_VECTORS {len(vecs)}")

if __name__ == "__main__": main()