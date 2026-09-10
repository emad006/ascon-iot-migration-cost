#pragma once
#include <stddef.h>

#define BENCH_NUM_WARMUP   32
#define BENCH_NUM_ITER     1000
#define BENCH_MAX_PAYLOAD  4096
#define BENCH_TASK_STACK_BYTES       65536   /* ML-KEM task only — matches the value Phase 1 proved sufficient */
#define BENCH_TASK_STACK_BYTES_SMALL 16384   /* Ascon + AES-GCM tasks — plenty of headroom, smaller footprint
                                                 so the previous task's stack has less to reclaim before the
                                                 next xTaskCreatePinnedToCore call */
#define BENCH_TASK_PRIORITY    5

static const size_t BENCH_PAYLOAD_SIZES[] = {16, 32, 64, 128, 256, 1024, 4096};
#define BENCH_NUM_PAYLOADS (sizeof(BENCH_PAYLOAD_SIZES)/sizeof(BENCH_PAYLOAD_SIZES[0]))

/* Associated data: models the MQTT topic string. Fixed for the whole sweep
 * per §3.2's gotcha — never let this vary across payload sizes. */
static const char BENCH_AD[] = "iot/gw01/telemetry";
#define BENCH_AD_LEN ((unsigned)(sizeof(BENCH_AD) - 1))

#if CONFIG_IDF_TARGET_ESP32
  #define BENCH_BOARD "esp32"
#elif CONFIG_IDF_TARGET_ESP32C3
  #define BENCH_BOARD "esp32c3"
#else
  #error "bench harness only supports esp32 / esp32c3"
#endif

/* AES impl label — derived from what's actually enabled, not a hand-maintained
 * constant. Verified against ESP-IDF's own mbedTLS Kconfig and this project's
 * generated sdkconfig for both boards (see Step 0 write-up): neither ESP32 nor
 * ESP32-C3 ever hardware-accelerates GHASH in this mbedTLS port, so there is no
 * honest "fully hw" state to report here — only "AES block hw, GHASH sw" or
 * pure software. */
#if CONFIG_MBEDTLS_HARDWARE_AES
  #define AES_IMPL_LABEL "hw_aes_sw_ghash"
#else
  #define AES_IMPL_LABEL "sw"
#endif