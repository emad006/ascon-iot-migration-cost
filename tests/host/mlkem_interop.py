#!/usr/bin/env python3
import subprocess
from kyber_py.ml_kem import ML_KEM_512

CLI = "./tests/host/mlkem_cli"

def run(*args):
    out = subprocess.run([CLI, *args], capture_output=True, text=True, check=True)
    return out.stdout.strip().split("\n")

def main():
    ek_hex, dk_hex = run("keygen")
    ek = bytes.fromhex(ek_hex)
    key_py, ct = ML_KEM_512.encaps(ek)
    [key_c_hex] = run("decaps", dk_hex, ct.hex())
    assert key_py == bytes.fromhex(key_c_hex), "MISMATCH in direction A"
    print("A (C keygen -> py encaps -> C decaps): MATCH")

    ek2, dk2 = ML_KEM_512.keygen()
    ct_hex2, key_c_hex2 = run("encaps", ek2.hex())
    key_py2 = ML_KEM_512.decaps(dk2, bytes.fromhex(ct_hex2))
    assert bytes.fromhex(key_c_hex2) == key_py2, "MISMATCH in direction B"
    print("B (py keygen -> C encaps -> py decaps): MATCH")

if __name__ == "__main__":
    main()