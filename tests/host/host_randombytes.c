#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

int randombytes(uint8_t *out, size_t n) {
    for (size_t i = 0; i < n; i++) out[i] = (uint8_t)(rand() & 0xff);
    return 0;
}