#include "gf257.h"

uint16_t gf257_normalize(int64_t value) {
    int64_t normalized = value % (int64_t)GF257_MODULUS;

    if (normalized < 0) {
        normalized += (int64_t)GF257_MODULUS;
    }

    return (uint16_t)normalized;
}

uint16_t gf257_add(uint16_t a, uint16_t b) {
    return (uint16_t)((a + b) % GF257_MODULUS);
}

uint16_t gf257_sub(uint16_t a, uint16_t b) {
    return gf257_normalize((int32_t)a - (int32_t)b);
}

uint16_t gf257_mul(uint16_t a, uint16_t b) {
    return (uint16_t)((a * b) % GF257_MODULUS);
}

uint16_t gf257_pow(uint16_t base, uint16_t exponent) {
    uint16_t result = 1;
    uint16_t factor = (uint16_t)(base % GF257_MODULUS);

    while (exponent > 0) {
        if ((exponent & 1u) != 0u) {
            result = gf257_mul(result, factor);
        }

        factor = gf257_mul(factor, factor);
        exponent = (uint16_t)(exponent >> 1u);
    }

    return result;
}

bool gf257_inverse(uint16_t value, uint16_t *out) {
    if (out == 0 || value == 0) {
        return false;
    }

    for (uint16_t candidate = 1; candidate < GF257_MODULUS; candidate++) {
        if (gf257_mul(value, candidate) == 1) {
            *out = candidate;
            return true;
        }
    }

    return false;
}

bool gf257_div(uint16_t numerator, uint16_t denominator, uint16_t *out) {
    uint16_t denominator_inverse;

    if (out == 0 || !gf257_inverse(denominator, &denominator_inverse)) {
        return false;
    }

    *out = gf257_mul(numerator, denominator_inverse);
    return true;
}
