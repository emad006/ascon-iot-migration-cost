#include <stdio.h>
#include <string.h>
#include "ascon_variants.h"
#include "kat_data/aead128_opt32_vectors.h"
#include "kat_data/aead128_opt64_vectors.h"
#include "kat_data/ascon128_opt32_vectors.h"
#include "kat_data/ascon128_opt64_vectors.h"
#include "kat_data/ascon128a_opt32_vectors.h"
#include "kat_data/ascon128a_opt64_vectors.h"

/* Runs one variant's whole KAT file: for every vector, encrypts pt/ad and checks
 * the ciphertext+tag matches the KAT's ct exactly, then decrypts that ct back and
 * checks it recovers the original pt. A vector only counts as passing if both
 * directions match. */
#define RUN_KAT_SUITE(LABEL, ENCFN, DECFN, VECARRAY, NUMVECS, TOTAL, FAILS)             \
    do {                                                                                \
        int suite_fails = 0;                                                            \
        for (int i = 0; i < (NUMVECS); i++) {                                           \
            const __typeof__((VECARRAY)[0]) *v = &(VECARRAY)[i];                        \
            unsigned char ct_buf[48];                                                   \
            unsigned long long ct_len = 0;                                              \
            unsigned char pt_buf[32];                                                    \
            unsigned long long pt_len = 0;                                              \
            int ok = 1;                                                                 \
            ENCFN(ct_buf, &ct_len, v->pt, (unsigned long long)v->pt_len,                \
                  v->ad, (unsigned long long)v->ad_len, NULL, v->nonce, v->key);         \
            if (ct_len != v->ct_len || memcmp(ct_buf, v->ct, v->ct_len) != 0) ok = 0;    \
            if (ok) {                                                                    \
                DECFN(pt_buf, &pt_len, NULL, v->ct, (unsigned long long)v->ct_len,       \
                      v->ad, (unsigned long long)v->ad_len, v->nonce, v->key);           \
                if (pt_len != v->pt_len ||                                               \
                    (v->pt_len > 0 && memcmp(pt_buf, v->pt, v->pt_len) != 0)) ok = 0;    \
            }                                                                            \
            if (!ok) {                                                                   \
                suite_fails++;                                                           \
                if (suite_fails <= 3)                                                    \
                    printf("  [FAIL] %s vector #%d (count=%d)\n", LABEL, i, v->count);   \
            }                                                                            \
        }                                                                                \
        printf("%-20s: %d/%d vectors passed\n", LABEL, (NUMVECS) - suite_fails, (NUMVECS)); \
        (TOTAL) += (NUMVECS);                                                            \
        (FAILS) += suite_fails;                                                          \
    } while (0)

void app_main(void)
{
    int total = 0, fails = 0;

    printf("\n=== PHASE 2 DEVICE KAT SELF-TEST ===\n\n");

    RUN_KAT_SUITE("ascon_aead128_opt32", ascon_aead128_opt32_encrypt, ascon_aead128_opt32_decrypt,
                  aead128_opt32_vectors, AEAD128_OPT32_NUM_VECTORS, total, fails);
    RUN_KAT_SUITE("ascon_aead128_opt64", ascon_aead128_opt64_encrypt, ascon_aead128_opt64_decrypt,
                  aead128_opt64_vectors, AEAD128_OPT64_NUM_VECTORS, total, fails);
    RUN_KAT_SUITE("ascon_128a_opt32", ascon_128a_opt32_encrypt, ascon_128a_opt32_decrypt,
                  ascon128a_opt32_vectors, ASCON128A_OPT32_NUM_VECTORS, total, fails);
    RUN_KAT_SUITE("ascon_128a_opt64", ascon_128a_opt64_encrypt, ascon_128a_opt64_decrypt,
                  ascon128a_opt64_vectors, ASCON128A_OPT64_NUM_VECTORS, total, fails);
    RUN_KAT_SUITE("ascon_128_opt32", ascon_128_opt32_encrypt, ascon_128_opt32_decrypt,
                  ascon128_opt32_vectors, ASCON128_OPT32_NUM_VECTORS, total, fails);
    RUN_KAT_SUITE("ascon_128_opt64", ascon_128_opt64_encrypt, ascon_128_opt64_decrypt,
                  ascon128_opt64_vectors, ASCON128_OPT64_NUM_VECTORS, total, fails);

    printf("\n");
    if (fails == 0) {
        printf("PHASE 2 DEVICE KAT: ALL PASS (%d/%d across 6 variants)\n", total, total);
    } else {
        printf("PHASE 2 DEVICE KAT: FAILED (%d/%d passed, %d failed)\n", total - fails, total, fails);
    }
}