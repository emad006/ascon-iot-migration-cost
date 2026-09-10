#include "aesgcm_adapter.h"

int aesgcm_adapter_setkey(mbedtls_gcm_context *ctx, const unsigned char *key) {
    mbedtls_gcm_init(ctx);
    return mbedtls_gcm_setkey(ctx, MBEDTLS_CIPHER_ID_AES, key, AESGCM_KEY_LEN * 8);
}

int aesgcm_adapter_encrypt(mbedtls_gcm_context *ctx,
    unsigned char *c, unsigned long long *clen,
    const unsigned char *m, unsigned long long mlen,
    const unsigned char *ad, unsigned long long adlen,
    const unsigned char *iv12)
{
    int rc = mbedtls_gcm_crypt_and_tag(ctx, MBEDTLS_GCM_ENCRYPT, (size_t)mlen,
        iv12, AESGCM_IV_LEN, ad, (size_t)adlen,
        m, c, AESGCM_TAG_LEN, c + mlen);
    *clen = mlen + AESGCM_TAG_LEN;
    return rc;
}

int aesgcm_adapter_decrypt(mbedtls_gcm_context *ctx,
    unsigned char *m, unsigned long long *mlen,
    const unsigned char *c, unsigned long long clen,
    const unsigned char *ad, unsigned long long adlen,
    const unsigned char *iv12)
{
    unsigned long long ptlen = clen - AESGCM_TAG_LEN;
    int rc = mbedtls_gcm_auth_decrypt(ctx, (size_t)ptlen,
        iv12, AESGCM_IV_LEN, ad, (size_t)adlen,
        c + ptlen, AESGCM_TAG_LEN, c, m);
    *mlen = ptlen;
    return rc;
}