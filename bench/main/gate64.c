// bench/main/gate64.c — Phase 4: Bachmann opt64 calibration gate
#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"   /* SemaphoreHandle_t, xSemaphoreGive */
#include "esp_cpu.h"
#include "bench_config.h"      /* BENCH_BOARD, GATE64_* */
#include "aead_iface.h"        /* aead_encrypt_fn / aead_decrypt_fn */
#include "ascon_variants.h"    /* ascon_128a_opt64_*, ascon_128_opt64_* */

extern SemaphoreHandle_t s_done;   /* defined in main.c, given back when this task finishes */

#define ASCON_KEY_LEN   16
#define ASCON_NONCE_LEN 16
#define ASCON_TAG_LEN   16

typedef struct {
    const char       *name;    /* matches Phase 3 naming: "Ascon128a" / "Ascon128" */
    aead_encrypt_fn    encrypt;
    aead_decrypt_fn    decrypt;
} gate64_variant_t;

/* Static globals, not task-stack locals — same discipline as bench_ascon_task
 * etc. in main.c, and the lesson from Phase 1's ML-KEM stack overflow. */
static uint8_t s_gate_pt[GATE64_PAYLOAD_BYTES];
static uint8_t s_gate_ct[GATE64_PAYLOAD_BYTES + ASCON_TAG_LEN];
static uint8_t s_gate_pt2[GATE64_PAYLOAD_BYTES];

static const uint8_t s_key[ASCON_KEY_LEN]     = {0};
static const uint8_t s_nonce[ASCON_NONCE_LEN] = {0};

static void gate64_run_variant(const gate64_variant_t *v)
{
    unsigned long long clen = 0, mlen = 0;

    /* One-time correctness self-check — not part of Bachmann's protocol.
     * Phase 2 validated these functions against 1089 KAT vectors, never at
     * 32 KB. Halt loudly rather than silently report a bogus fast number. */
    int rc_enc = v->encrypt(s_gate_ct, &clen, s_gate_pt, GATE64_PAYLOAD_BYTES,
                             NULL, GATE64_AD_BYTES, NULL, s_nonce, s_key);
    int rc_dec = v->decrypt(s_gate_pt2, &mlen, NULL, s_gate_ct, clen,
                             NULL, GATE64_AD_BYTES, s_nonce, s_key);
    if (rc_enc != 0 || rc_dec != 0 || mlen != GATE64_PAYLOAD_BYTES ||
        memcmp(s_gate_pt, s_gate_pt2, GATE64_PAYLOAD_BYTES) != 0) {
        printf("GATE64: FATAL self-check failed for %s (rc_enc=%d rc_dec=%d mlen=%llu)\n",
               v->name, rc_enc, rc_dec, mlen);
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }

    for (uint32_t w = 0; w < GATE64_WARMUP_CALLS; w++)
        v->encrypt(s_gate_ct, &clen, s_gate_pt, GATE64_PAYLOAD_BYTES,
                   NULL, GATE64_AD_BYTES, NULL, s_nonce, s_key);

    for (uint32_t iter = 0; iter < GATE64_SAMPLES; iter++) {
        uint32_t t0 = esp_cpu_get_cycle_count();
        for (uint32_t r = 0; r < GATE64_REPEATS; r++)
            v->encrypt(s_gate_ct, &clen, s_gate_pt, GATE64_PAYLOAD_BYTES,
                       NULL, GATE64_AD_BYTES, NULL, s_nonce, s_key);
        uint32_t enc_cycles = (esp_cpu_get_cycle_count() - t0) / GATE64_REPEATS;

        uint32_t t1 = esp_cpu_get_cycle_count();
        for (uint32_t r = 0; r < GATE64_REPEATS; r++)
            v->decrypt(s_gate_pt2, &mlen, NULL, s_gate_ct, clen,
                       NULL, GATE64_AD_BYTES, s_nonce, s_key);
        uint32_t dec_cycles = (esp_cpu_get_cycle_count() - t1) / GATE64_REPEATS;

        /* Same 8-field schema as the rest of bench/ — no re-printed header. */
        printf("%s,%s,opt64,encrypt,%lu,%lu,%lu,%lu\n",
               BENCH_BOARD, v->name, (unsigned long)GATE64_PAYLOAD_BYTES,
               (unsigned long)GATE64_AD_BYTES, (unsigned long)iter, (unsigned long)enc_cycles);
        printf("%s,%s,opt64,decrypt,%lu,%lu,%lu,%lu\n",
               BENCH_BOARD, v->name, (unsigned long)GATE64_PAYLOAD_BYTES,
               (unsigned long)GATE64_AD_BYTES, (unsigned long)iter, (unsigned long)dec_cycles);
    }

    printf("stack,%s,%s,opt64,gate,%lu,%u\n",
           BENCH_BOARD, v->name, (unsigned long)GATE64_PAYLOAD_BYTES,
           (unsigned)uxTaskGetStackHighWaterMark(NULL));
}

void gate64_task(void *pv)
{
    (void)pv;
    static const gate64_variant_t variants[] = {
        { "Ascon128a", ascon_128a_opt64_encrypt, ascon_128a_opt64_decrypt },
        { "Ascon128",  ascon_128_opt64_encrypt,  ascon_128_opt64_decrypt  },
    };
    for (size_t i = 0; i < sizeof(variants) / sizeof(variants[0]); i++)
        gate64_run_variant(&variants[i]);

    xSemaphoreGive(s_done);
    vTaskDelete(NULL);
}