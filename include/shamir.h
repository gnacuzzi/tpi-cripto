#ifndef SHAMIR_H
#define SHAMIR_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    SHAMIR_OK = 0,
    SHAMIR_ERR_ARG,
    SHAMIR_ERR_RANGE,
    SHAMIR_ERR_BUFFER,
    SHAMIR_ERR_INTERPOLATION
} ShamirError;

const char *shamir_error_string(ShamirError err);

size_t shamir_shadow_byte_count(size_t secret_len, int k);

ShamirError shamir_evaluate_block(uint16_t *coefficients, int k, int n,
                                  uint8_t *shadow_bytes);
ShamirError shamir_split(const uint8_t *secret, size_t secret_len,
                         int k, int n, uint16_t seed,
                         uint8_t *shadows, size_t shadow_len);
ShamirError shamir_recover(const uint8_t *shadows, const uint16_t *shadow_indices,
                           int k, size_t shadow_len, uint16_t seed,
                           uint8_t *secret_out, size_t secret_len);

#endif
