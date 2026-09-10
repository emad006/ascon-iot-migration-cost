#include <stdint.h>
#include <string.h>

static uint64_t s_state = 0x1234567890ABCDEFULL;
static uint64_t splitmix64_next(void) {
    uint64_t z = (s_state += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}
int randombytes(uint8_t *buf, size_t n) {
    while (n >= 8) { uint64_t r = splitmix64_next(); memcpy(buf, &r, 8); buf += 8; n -= 8; }
    if (n) { uint64_t r = splitmix64_next(); memcpy(buf, &r, n); }
    return 0;
}