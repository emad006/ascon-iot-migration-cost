// bench/main/main.c
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mbedtls/gcm.h"
#include "ascon_variants.h"

#define MLK_CONFIG_FILE "my_mlkem_config.h"
#include "mlkem_native.h"

void app_main(void) {
    uint8_t key[16]={0}, npub[16]={0}, ad[8]={0}, pt[16]={0};
    uint8_t ct[32]; unsigned long long ctlen;

    ascon_aead128_opt32_encrypt(ct,&ctlen,pt,sizeof(pt),ad,sizeof(ad),NULL,npub,key);
    ascon_aead128_opt64_encrypt(ct,&ctlen,pt,sizeof(pt),ad,sizeof(ad),NULL,npub,key);
    ascon_128a_opt32_encrypt(ct,&ctlen,pt,sizeof(pt),ad,sizeof(ad),NULL,npub,key);
    ascon_128a_opt64_encrypt(ct,&ctlen,pt,sizeof(pt),ad,sizeof(ad),NULL,npub,key);
    ascon_128_opt32_encrypt(ct,&ctlen,pt,sizeof(pt),ad,sizeof(ad),NULL,npub,key);
    ascon_128_opt64_encrypt(ct,&ctlen,pt,sizeof(pt),ad,sizeof(ad),NULL,npub,key);
    printf("Ascon: all six variants linked and callable.\n");

    mbedtls_gcm_context gcm;
    mbedtls_gcm_init(&gcm);
    mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, key, 128);
    uint8_t gcm_iv[12]={0}, gcm_tag[16];
    mbedtls_gcm_crypt_and_tag(&gcm, MBEDTLS_GCM_ENCRYPT, sizeof(pt), gcm_iv, sizeof(gcm_iv),
                               ad, sizeof(ad), pt, ct, sizeof(gcm_tag), gcm_tag);
    mbedtls_gcm_free(&gcm);
    printf("AES-128-GCM (mbedTLS): callable.\n");

    uint8_t pk[CRYPTO_PUBLICKEYBYTES], sk[CRYPTO_SECRETKEYBYTES], kct[CRYPTO_CIPHERTEXTBYTES];
    uint8_t ss_a[CRYPTO_BYTES], ss_b[CRYPTO_BYTES];
    crypto_kem_keypair(pk, sk);
    crypto_kem_enc(kct, ss_a, pk);
    crypto_kem_dec(ss_b, kct, sk);
    printf("ML-KEM-512: %s\n", memcmp(ss_a, ss_b, CRYPTO_BYTES)==0 ? "shared secrets matched" : "MISMATCH");

    printf("PHASE 1 SMOKE TEST: all components linked cleanly.\n");
    while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
}