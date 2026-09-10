idf_component_register(
    SRCS         "../../vendor/ascon-new/crypto_aead/asconaead128/opt64/aead.c"
                 "../../vendor/ascon-new/crypto_aead/asconaead128/opt64/permutations.c"
    INCLUDE_DIRS "../../vendor/ascon-new/crypto_aead/asconaead128/opt64"
                 "../../vendor/ascon-new/tests"
)
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    crypto_aead_encrypt=ascon_aead128_opt64_encrypt
    crypto_aead_decrypt=ascon_aead128_opt64_decrypt
    ascon_aead_encrypt=ascon_aead128_opt64_core_encrypt
    ascon_aead_decrypt=ascon_aead128_opt64_core_decrypt
)