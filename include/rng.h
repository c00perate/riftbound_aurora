#ifndef RNG_H
#define RNG_H

#include <stdint.h>
#include "config.h"

/* ---------------------- RNG (xorshift64*) ---------------------- */
static inline uint64_t rng_next(RNG *r) {
    uint64_t x = r->state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    r->state = x;
    return x * 0x2545F4914F6CDD1DULL;
}

static inline int rng_int(RNG *r, int n) { /* uniform in [0,n) */
    return (int)(rng_next(r) % (uint64_t)n);
}

static inline double rng_double(RNG *r) {
    return (double)(rng_next(r) >> 11) * (1.0 / 9007199254740992.0);
}

void shuffle_array(CardType *arr, int n, RNG *rng);

#endif