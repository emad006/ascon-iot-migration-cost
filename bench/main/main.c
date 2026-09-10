#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "esp_cpu.h"
#include "esp_system.h"          /* esp_get_free_heap_size() — Step 1.1 hang fix */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "bench_config.h"
#include "aead_iface.h"
#include "ascon_variants.h"     /* from crypto_common — Phase 1 */
#include "aesgcm_adapter.h"     /* new, Step 1.3 */

/* Real mlkem-native API — must define MLK_CONFIG_FILE before including
 * mlkem_native.h in THIS translation unit, exactly like Phase 1's main.c
 * does. It's only a PRIVATE compile definition on the mlkem512 component
 * itself, so it does not propagate here automatically — without this line,
 * mlkem_native.h silently falls back to the default config (ML-KEM-768,
 * not 512). This exposes crypto_kem_keypair / crypto_kem_enc / crypto_kem_dec
 * and the CRYPTO_* size macros used below. */
#define MLK_CONFIG_FILE "my_mlkem_config.h"
#include "mlkem_native.h"

static SemaphoreHandle_t s_done;

typedef struct {
    const char *name;
    aead_encrypt_fn encrypt;
    aead_decrypt_fn decrypt;
} ascon_variant_t;

/* prng_bench's actual signature — note it returns int, not void. */
extern int randombytes(uint8_t *buf, size_t len);

static void bench_ascon_task(void *arg) {
    const ascon_variant_t *v = (const ascon_variant_t *)arg;

    static uint8_t pt[BENCH_MAX_PAYLOAD];
    static uint8_t ct[BENCH_MAX_PAYLOAD + 16];
    static uint8_t pt_out[BENCH_MAX_PAYLOAD];
    uint8_t key[16], nonce[16];
    randombytes(key, sizeof(key));

    for (size_t p = 0; p < BENCH_NUM_PAYLOADS; p++) {
        size_t plen = BENCH_PAYLOAD_SIZES[p];
        randombytes(pt, plen);
        randombytes(nonce, sizeof(nonce));
        unsigned long long clen, mlen;

        for (int w = 0; w < BENCH_NUM_WARMUP; w++)
            v->encrypt(ct, &clen, pt, plen, (const unsigned char*)BENCH_AD, BENCH_AD_LEN, NULL, nonce, key);

        for (int i = 0; i < BENCH_NUM_ITER; i++) {
            uint32_t t0 = esp_cpu_get_cycle_count();
            v->encrypt(ct, &clen, pt, plen, (const unsigned char*)BENCH_AD, BENCH_AD_LEN, NULL, nonce, key);
            uint32_t t1 = esp_cpu_get_cycle_count();
            printf("%s,%s,opt32,encrypt,%u,%u,%d,%u\n",
                   BENCH_BOARD, v->name, (unsigned)plen, BENCH_AD_LEN, i, (unsigned)(t1 - t0));
        }
        printf("stack,%s,%s,opt32,encrypt,%u,%u\n",
               BENCH_BOARD, v->name, (unsigned)plen, (unsigned)uxTaskGetStackHighWaterMark(NULL));

        /* one untimed encrypt -> valid ciphertext to decrypt repeatedly */
        v->encrypt(ct, &clen, pt, plen, (const unsigned char*)BENCH_AD, BENCH_AD_LEN, NULL, nonce, key);

        for (int w = 0; w < BENCH_NUM_WARMUP; w++)
            v->decrypt(pt_out, &mlen, NULL, ct, clen, (const unsigned char*)BENCH_AD, BENCH_AD_LEN, nonce, key);

        for (int i = 0; i < BENCH_NUM_ITER; i++) {
            uint32_t t0 = esp_cpu_get_cycle_count();
            v->decrypt(pt_out, &mlen, NULL, ct, clen, (const unsigned char*)BENCH_AD, BENCH_AD_LEN, nonce, key);
            uint32_t t1 = esp_cpu_get_cycle_count();
            printf("%s,%s,opt32,decrypt,%u,%u,%d,%u\n",
                   BENCH_BOARD, v->name, (unsigned)plen, BENCH_AD_LEN, i, (unsigned)(t1 - t0));
        }
        printf("stack,%s,%s,opt32,decrypt,%u,%u\n",
               BENCH_BOARD, v->name, (unsigned)plen, (unsigned)uxTaskGetStackHighWaterMark(NULL));
    }

    xSemaphoreGive(s_done);
    vTaskDelete(NULL);
}

static void bench_aesgcm_task(void *arg) {
    (void)arg;
    static uint8_t pt[BENCH_MAX_PAYLOAD];
    static uint8_t ct[BENCH_MAX_PAYLOAD + AESGCM_TAG_LEN];
    static uint8_t pt_out[BENCH_MAX_PAYLOAD];
    uint8_t key[AESGCM_KEY_LEN], iv[AESGCM_IV_LEN];
    randombytes(key, sizeof(key));

    mbedtls_gcm_context ctx;
    aesgcm_adapter_setkey(&ctx, key);   /* once, outside every timed loop */

    for (size_t p = 0; p < BENCH_NUM_PAYLOADS; p++) {
        size_t plen = BENCH_PAYLOAD_SIZES[p];
        randombytes(pt, plen);
        randombytes(iv, sizeof(iv));
        unsigned long long clen, mlen;

        for (int w = 0; w < BENCH_NUM_WARMUP; w++)
            aesgcm_adapter_encrypt(&ctx, ct, &clen, pt, plen, (const unsigned char*)BENCH_AD, BENCH_AD_LEN, iv);

        for (int i = 0; i < BENCH_NUM_ITER; i++) {
            uint32_t t0 = esp_cpu_get_cycle_count();
            aesgcm_adapter_encrypt(&ctx, ct, &clen, pt, plen, (const unsigned char*)BENCH_AD, BENCH_AD_LEN, iv);
            uint32_t t1 = esp_cpu_get_cycle_count();
            printf("%s,AES128GCM,%s,encrypt,%u,%u,%d,%u\n",
                   BENCH_BOARD, AES_IMPL_LABEL, (unsigned)plen, BENCH_AD_LEN, i, (unsigned)(t1 - t0));
        }
        printf("stack,%s,AES128GCM,%s,encrypt,%u,%u\n",
               BENCH_BOARD, AES_IMPL_LABEL, (unsigned)plen, (unsigned)uxTaskGetStackHighWaterMark(NULL));

        aesgcm_adapter_encrypt(&ctx, ct, &clen, pt, plen, (const unsigned char*)BENCH_AD, BENCH_AD_LEN, iv);

        for (int w = 0; w < BENCH_NUM_WARMUP; w++)
            aesgcm_adapter_decrypt(&ctx, pt_out, &mlen, ct, clen, (const unsigned char*)BENCH_AD, BENCH_AD_LEN, iv);

        for (int i = 0; i < BENCH_NUM_ITER; i++) {
            uint32_t t0 = esp_cpu_get_cycle_count();
            aesgcm_adapter_decrypt(&ctx, pt_out, &mlen, ct, clen, (const unsigned char*)BENCH_AD, BENCH_AD_LEN, iv);
            uint32_t t1 = esp_cpu_get_cycle_count();
            printf("%s,AES128GCM,%s,decrypt,%u,%u,%d,%u\n",
                   BENCH_BOARD, AES_IMPL_LABEL, (unsigned)plen, BENCH_AD_LEN, i, (unsigned)(t1 - t0));
        }
        printf("stack,%s,AES128GCM,%s,decrypt,%u,%u\n",
               BENCH_BOARD, AES_IMPL_LABEL, (unsigned)plen, (unsigned)uxTaskGetStackHighWaterMark(NULL));
    }

    mbedtls_gcm_free(&ctx);
    xSemaphoreGive(s_done);
    vTaskDelete(NULL);
}

static void bench_mlkem_task(void *arg) {
    (void)arg;
    static uint8_t pk[CRYPTO_PUBLICKEYBYTES];
    static uint8_t sk[CRYPTO_SECRETKEYBYTES];
    static uint8_t ct[CRYPTO_CIPHERTEXTBYTES];
    static uint8_t ss_a[CRYPTO_BYTES], ss_b[CRYPTO_BYTES];

    for (int w = 0; w < BENCH_NUM_WARMUP; w++) crypto_kem_keypair(pk, sk);
    for (int i = 0; i < BENCH_NUM_ITER; i++) {
        uint32_t t0 = esp_cpu_get_cycle_count();
        crypto_kem_keypair(pk, sk);
        uint32_t t1 = esp_cpu_get_cycle_count();
        printf("%s,MLKEM512,clean,keygen,0,0,%d,%u\n", BENCH_BOARD, i, (unsigned)(t1 - t0));
    }
    printf("stack,%s,MLKEM512,clean,keygen,0,%u\n", BENCH_BOARD, (unsigned)uxTaskGetStackHighWaterMark(NULL));

    for (int w = 0; w < BENCH_NUM_WARMUP; w++) crypto_kem_enc(ct, ss_a, pk);
    for (int i = 0; i < BENCH_NUM_ITER; i++) {
        uint32_t t0 = esp_cpu_get_cycle_count();
        crypto_kem_enc(ct, ss_a, pk);
        uint32_t t1 = esp_cpu_get_cycle_count();
        printf("%s,MLKEM512,clean,encaps,0,0,%d,%u\n", BENCH_BOARD, i, (unsigned)(t1 - t0));
    }
    printf("stack,%s,MLKEM512,clean,encaps,0,%u\n", BENCH_BOARD, (unsigned)uxTaskGetStackHighWaterMark(NULL));

    for (int w = 0; w < BENCH_NUM_WARMUP; w++) crypto_kem_dec(ss_b, ct, sk);
    for (int i = 0; i < BENCH_NUM_ITER; i++) {
        uint32_t t0 = esp_cpu_get_cycle_count();
        crypto_kem_dec(ss_b, ct, sk);
        uint32_t t1 = esp_cpu_get_cycle_count();
        printf("%s,MLKEM512,clean,decaps,0,0,%d,%u\n", BENCH_BOARD, i, (unsigned)(t1 - t0));
    }
    printf("stack,%s,MLKEM512,clean,decaps,0,%u\n", BENCH_BOARD, (unsigned)uxTaskGetStackHighWaterMark(NULL));

    xSemaphoreGive(s_done);
    vTaskDelete(NULL);
}

static void launch_and_wait(TaskFunction_t fn, void *arg, uint32_t stack_bytes) {
    /* Give the idle task a moment to reclaim the previous bench task's stack
     * (vTaskDelete(NULL) marks it for deletion but doesn't free it immediately —
     * app_main runs at a higher priority than idle, so without this delay the
     * next xTaskCreatePinnedToCore can be asked to allocate while the old
     * stack's memory is still pending reclamation). */
    vTaskDelay(pdMS_TO_TICKS(10));
    printf("# free heap before launch: %u\n", (unsigned)esp_get_free_heap_size());

    TaskHandle_t handle = NULL;
    BaseType_t ok = xTaskCreatePinnedToCore(fn, "bench", stack_bytes, arg, BENCH_TASK_PRIORITY, &handle, 0);
    if (ok != pdPASS) {
        printf("BENCH: FATAL — xTaskCreatePinnedToCore failed (stack_bytes=%u, free_heap=%u)\n",
               (unsigned)stack_bytes, (unsigned)esp_get_free_heap_size());
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }
    xSemaphoreTake(s_done, portMAX_DELAY);
}

void app_main(void) {
    s_done = xSemaphoreCreateBinary();

    printf("board,algorithm,impl,operation,payload_bytes,ad_bytes,iteration,cycles\n");

    static const ascon_variant_t variants[] = {
        { "AsconAEAD128", ascon_aead128_opt32_encrypt, ascon_aead128_opt32_decrypt },
        { "Ascon128a",    ascon_128a_opt32_encrypt,    ascon_128a_opt32_decrypt },
        { "Ascon128",     ascon_128_opt32_encrypt,     ascon_128_opt32_decrypt },
    };

#if !CONFIG_BENCH_AES_SWEEP_ONLY
    for (size_t i = 0; i < 3; i++)
        launch_and_wait(bench_ascon_task, (void *)&variants[i], BENCH_TASK_STACK_BYTES_SMALL);

    launch_and_wait(bench_mlkem_task, NULL, BENCH_TASK_STACK_BYTES);
#endif

    launch_and_wait(bench_aesgcm_task, NULL, BENCH_TASK_STACK_BYTES_SMALL);

    printf("BENCH: matrix complete, board=%s aes_impl=%s\n", BENCH_BOARD, AES_IMPL_LABEL);
    while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
}