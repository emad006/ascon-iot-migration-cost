idf_component_register(
    SRCS         "main.c" "gate64.c"
    INCLUDE_DIRS "."
    REQUIRES     ascon_aead128_opt32 ascon_aead128_opt64
                 ascon_128a_opt32 ascon_128a_opt64
                 ascon_128_opt32 ascon_128_opt64
                 mlkem512 crypto_common mbedtls
)