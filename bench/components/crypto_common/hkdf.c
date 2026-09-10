#include "mbedtls/hkdf.h"
#include "mbedtls/md.h"
#include "crypto_common.h"

int crypto_common_hkdf_sha256(const uint8_t *ikm, size_t ikm_len,
                               const uint8_t *salt, size_t salt_len,
                               const uint8_t *info, size_t info_len,
                               uint8_t *okm, size_t okm_len) {
    const mbedtls_md_info_t *md = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    return mbedtls_hkdf(md, salt, salt_len, ikm, ikm_len, info, info_len, okm, okm_len);
}