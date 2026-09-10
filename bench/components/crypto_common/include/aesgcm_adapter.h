#pragma once
#include "mbedtls/gcm.h"
#include <stddef.h>

#define AESGCM_KEY_LEN 16
#define AESGCM_IV_LEN  12
#define AESGCM_TAG_LEN 16

int aesgcm_adapter_setkey(mbedtls_gcm_context *ctx, const unsigned char *key);

int aesgcm_adapter_encrypt(mbedtls_gcm_context *ctx,
    unsigned char *c, unsigned long long *clen,
    const unsigned char *m, unsigned long long mlen,
    const unsigned char *ad, unsigned long long adlen,
    const unsigned char *iv12);

int aesgcm_adapter_decrypt(mbedtls_gcm_context *ctx,
    unsigned char *m, unsigned long long *mlen,
    const unsigned char *c, unsigned long long clen,
    const unsigned char *ad, unsigned long long adlen,
    const unsigned char *iv12);