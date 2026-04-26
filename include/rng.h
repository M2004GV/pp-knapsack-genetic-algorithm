#ifndef RNG_H
#define RNG_H

static inline unsigned int xorshift32(unsigned int *state) {
    unsigned int x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return (*state = x);
}

static inline int rand_int_ts(int min, int max, unsigned int *seed) {
    return min + (int)(xorshift32(seed) % (unsigned int)(max - min + 1));
}

static inline double rand_double_ts(unsigned int *seed) {
    return (double)xorshift32(seed) / (double)0xFFFFFFFFU;
}

#endif