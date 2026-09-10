idf_component_register(
    SRCS         "../../vendor/ascon-new/crypto_aead/asconaead128/opt32/aead.c"
                 "../../vendor/ascon-new/crypto_aead/asconaead128/opt32/permutations.c"
    INCLUDE_DIRS "../../vendor/ascon-new/crypto_aead/asconaead128/opt32"
                 "../../vendor/ascon-new/tests"
)
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    crypto_aead_encrypt=ascon_aead128_opt32_encrypt
    crypto_aead_decrypt=ascon_aead128_opt32_decrypt
    ascon_aead_encrypt=ascon_aead128_opt32_core_encrypt
    ascon_aead_decrypt=ascon_aead128_opt32_core_decrypt
)