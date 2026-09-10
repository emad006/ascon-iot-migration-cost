#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "mlkem_native.h"   /* the real public header your bench/main/main.c already
                               includes — declares crypto_kem_keypair/enc/dec and
                               CRYPTO_PUBLICKEYBYTES/SECRETKEYBYTES/CIPHERTEXTBYTES/BYTES */

static void print_hex(const uint8_t *b, size_t n) {
    for (size_t i = 0; i < n; i++) printf("%02x", b[i]);
    printf("\n");
}
static void hex_to_bin(const char *hex, uint8_t *out, size_t n) {
    for (size_t i = 0; i < n; i++) sscanf(hex + 2*i, "%2hhx", &out[i]);
}

int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr, "usage: keygen | encaps <ek_hex> | decaps <dk_hex> <ct_hex>\n"); return 1; }

    if (!strcmp(argv[1], "keygen")) {
        uint8_t ek[CRYPTO_PUBLICKEYBYTES], dk[CRYPTO_SECRETKEYBYTES];
        crypto_kem_keypair(ek, dk);
        print_hex(ek, sizeof ek);
        print_hex(dk, sizeof dk);
    } else if (!strcmp(argv[1], "encaps")) {
        uint8_t ek[CRYPTO_PUBLICKEYBYTES], ct[CRYPTO_CIPHERTEXTBYTES], key[CRYPTO_BYTES];
        hex_to_bin(argv[2], ek, sizeof ek);
        crypto_kem_enc(ct, key, ek);
        print_hex(ct, sizeof ct);
        print_hex(key, sizeof key);
    } else if (!strcmp(argv[1], "decaps")) {
        uint8_t dk[CRYPTO_SECRETKEYBYTES], ct[CRYPTO_CIPHERTEXTBYTES], key[CRYPTO_BYTES];
        hex_to_bin(argv[2], dk, sizeof dk);
        hex_to_bin(argv[3], ct, sizeof ct);
        crypto_kem_dec(key, ct, dk);
        print_hex(key, sizeof key);
    } else {
        fprintf(stderr, "unknown mode: %s\n", argv[1]);
        return 1;
    }
    return 0;
}