idf_component_register(
    SRC_DIRS     "../../vendor/mlkem-native/mlkem/src"
                 "../../vendor/mlkem-native/mlkem/src/fips202"
    INCLUDE_DIRS "."
                 "../../vendor/mlkem-native/mlkem/src"
                 "../../vendor/mlkem-native/mlkem"
    REQUIRES     prng_bench
)
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    MLK_CONFIG_FILE="my_mlkem_config.h"
)