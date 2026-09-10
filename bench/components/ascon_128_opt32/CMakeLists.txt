idf_component_register(
    SRCS         "../../vendor/ascon-old/crypto_aead/ascon128v12/opt32/aead.c"
                 "../../vendor/ascon-old/crypto_aead/ascon128v12/opt32/permutations.c"
    INCLUDE_DIRS "../../vendor/ascon-old/crypto_aead/ascon128v12/opt32"
                 "../../vendor/ascon-old/tests"
)
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    crypto_aead_encrypt=ascon_128_opt32_encrypt
    crypto_aead_decrypt=ascon_128_opt32_decrypt
    ascon_aead_encrypt=ascon_128_opt32_core_encrypt
    ascon_aead_decrypt=ascon_128_opt32_core_decrypt
)