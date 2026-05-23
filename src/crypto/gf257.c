#include "gf257.h"

uint16_t inverse_matrix[256] = {
    0,   1,   0, 171,   0, 205,   0, 183,   0,  57,   0, 163,   0, 197,   0, 239,   0, 241,   0,  27,   0,  61,   0, 167,   0,  41,   0,  19,   0,  53,   0, 223,
    0, 225,   0, 139,   0, 173,   0, 151,   0,  25,   0, 131,   0, 165,   0, 207,   0, 209,   0, 251,   0,  29,   0, 135,   0,   9,   0, 243,   0,  21,   0, 191,
    0, 193,   0, 107,   0, 141,   0, 119,   0, 249,   0,  99,   0, 133,   0, 175,   0, 177,   0, 219,   0, 253,   0, 103,   0, 233,   0, 211,   0, 245,   0, 159,
    0, 161,   0,  75,   0, 109,   0,  87,   0, 217,   0,  67,   0, 101,   0, 143,   0, 145,   0, 187,   0, 221,   0,  71,   0, 201,   0, 179,   0, 213,   0, 127,
    0, 129,   0,  43,   0,  77,   0,  55,   0, 185,   0,  35,   0,  69,   0, 111,   0, 113,   0, 155,   0, 189,   0,  39,   0, 169,   0, 147,   0, 181,   0,  95,
    0,  97,   0,  11,   0,  45,   0,  23,   0, 153,   0,   3,   0,  37,   0,  79,   0,  81,   0, 123,   0, 157,   0,   7,   0, 137,   0, 115,   0, 149,   0,  63,
    0,  65,   0, 235,   0,  13,   0, 247,   0, 121,   0, 227,   0,   5,   0,  47,   0,  49,   0,  91,   0, 125,   0, 231,   0, 105,   0,  83,   0, 117,   0,  31,
    0,  33,   0, 203,   0, 237,   0, 215,   0,  89,   0, 195,   0, 229,   0,  15,   0,  17,   0,  59,   0,  93,   0, 199,   0,  73,   0,  51,   0,  85,   0, 255
};

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
    if (!out) {
        return false;
    }
    uint16_t aux = value % 256;
    if(inverse_matrix[aux]){
        *out = inverse_matrix[aux];
        return true;
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
