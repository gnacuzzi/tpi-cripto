#ifndef GF257_H
#define GF257_H

#include <stdbool.h>
#include <stdint.h>

#define GF257_MODULUS 257u

uint16_t gf257_normalize(int64_t value);
uint16_t gf257_add(uint16_t a, uint16_t b);
uint16_t gf257_sub(uint16_t a, uint16_t b);
uint16_t gf257_mul(uint16_t a, uint16_t b);
uint16_t gf257_pow(uint16_t base, uint16_t exponent);
bool gf257_inverse(uint16_t value, uint16_t *out);
bool gf257_div(uint16_t numerator, uint16_t denominator, uint16_t *out);

#endif
