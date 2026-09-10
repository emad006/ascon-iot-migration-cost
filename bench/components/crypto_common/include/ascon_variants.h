#pragma once
#include <stddef.h>

#define ASCON_VARIANT_DECL(prefix) \
    int prefix##_encrypt(unsigned char *c, unsigned long long *clen, \
        const unsigned char *m, unsigned long long mlen, \
        const unsigned char *ad, unsigned long long adlen, \
        const unsigned char *nsec, const unsigned char *npub, const unsigned char *k); \
    int prefix##_decrypt(unsigned char *m, unsigned long long *mlen, \
        unsigned char *nsec, \
        const unsigned char *c, unsigned long long clen, \
        const unsigned char *ad, unsigned long long adlen, \
        const unsigned char *npub, const unsigned char *k);

ASCON_VARIANT_DECL(ascon_aead128_opt32)
ASCON_VARIANT_DECL(ascon_aead128_opt64)
ASCON_VARIANT_DECL(ascon_128a_opt32)
ASCON_VARIANT_DECL(ascon_128a_opt64)
ASCON_VARIANT_DECL(ascon_128_opt32)
ASCON_VARIANT_DECL(ascon_128_opt64)