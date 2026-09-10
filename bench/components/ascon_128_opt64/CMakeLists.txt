idf_component_register(
    SRCS         "../../vendor/ascon-old/crypto_aead/ascon128v12/opt64/aead.c"
                 "../../vendor/ascon-old/crypto_aead/ascon128v12/opt64/permutations.c"
    INCLUDE_DIRS "../../vendor/ascon-old/crypto_aead/ascon128v12/opt64"
                 "../../vendor/ascon-old/tests"
)
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    crypto_aead_encrypt=ascon_128_opt64_encrypt
    crypto_aead_decrypt=ascon_128_opt64_decrypt
    ascon_aead_encrypt=ascon_128_opt64_core_encrypt
    ascon_aead_decrypt=ascon_128_opt64_core_decrypt
)