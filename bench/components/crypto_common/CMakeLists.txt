idf_component_register(
    SRCS         "hkdf.c" "aesgcm_adapter.c"
    INCLUDE_DIRS "include"
    REQUIRES     mbedtls
)