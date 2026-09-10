#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "crypto_common.h"

static int hex_eq(const uint8_t *b, const char *hex, size_t n) {
    for (size_t i = 0; i < n; i++) {
        char buf[3]; sprintf(buf, "%02x", b[i]);
        if (buf[0] != hex[2*i] || buf[1] != hex[2*i+1]) return 0;
    }
    return 1;
}
static void hex_to_bin(const char *hex, uint8_t *out, size_t n) {
    for (size_t i = 0; i < n; i++) sscanf(hex + 2*i, "%2hhx", &out[i]);
}

int main(void) {
    uint8_t ikm[22], salt[13], info[10], okm[42];
    hex_to_bin("0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b", ikm, 22);
    hex_to_bin("000102030405060708090a0b0c", salt, 13);
    hex_to_bin("f0f1f2f3f4f5f6f7f8f9", info, 10);

    const char *expected_okm =
        "3cb25f25faacd57a90434f64d0362f2a2d2d0a90cf1a5a4c5db02d56ecc4c5bf34007208d5b887185865";

    int rc = crypto_common_hkdf_sha256(ikm, sizeof ikm, salt, sizeof salt,
                                        info, sizeof info, okm, sizeof okm);

    if (rc != 0) { printf("crypto_common_hkdf_sha256 error %d\n", rc); return 1; }
    if (hex_eq(okm, expected_okm, sizeof okm)) {
        printf("RFC 5869 TC1 (via crypto_common wrapper): PASS\n");
        return 0;
    }
    printf("RFC 5869 TC1 (via crypto_common wrapper): FAIL\n");
    for (size_t i = 0; i < sizeof okm; i++) printf("%02x", okm[i]);
    printf("\n");
    return 1;
}