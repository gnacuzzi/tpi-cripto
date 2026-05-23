#include "shamir.h"

#include "gf257.h"
#include "lagrange.h"
#include "permutation_table.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SHAMIR_MIN_K 2
#define SHAMIR_MAX_K 10
#define SHAMIR_MAX_N 256

const char *shamir_error_string(ShamirError err) {
    switch (err) {
    case SHAMIR_OK:
        return "ok";
    case SHAMIR_ERR_ARG:
        return "invalid argument";
    case SHAMIR_ERR_RANGE:
        return "invalid k or n";
    case SHAMIR_ERR_BUFFER:
        return "invalid buffer length";
    case SHAMIR_ERR_INTERPOLATION:
        return "interpolation failed";
    }
    return "unknown Shamir error";
}

size_t shamir_shadow_byte_count(size_t secret_len, int k) {
    if (k <= 0 || secret_len == 0) {
        return 0;
    }
    return (secret_len + (size_t)k - 1U) / (size_t)k;
}

static bool valid_k_n(int k, int n) {
    return k >= SHAMIR_MIN_K && k <= SHAMIR_MAX_K
        && n >= k && n <= SHAMIR_MAX_N;
}

static uint16_t evaluate_polynomial(const uint16_t *coefficients, int k, uint16_t x) {
    uint16_t y = coefficients[k - 1];

    for (int i = k - 2; i >= 0; i--) {
        y = gf257_mul(y, x);
        y = gf257_add(y, coefficients[i]);
    }

    return y;
}

static bool decrement_first_nonzero(uint16_t *coefficients, int k) {
    for (int i = 0; i < k; i++) {
        if (coefficients[i] != 0) {
            coefficients[i] = gf257_sub(coefficients[i], 1);
            return true;
        }
    }
    return false;
}

ShamirError shamir_evaluate_block(uint16_t *coefficients, int k, int n,
                                  uint8_t *shadow_bytes) {
    if (coefficients == NULL || shadow_bytes == NULL) {
        return SHAMIR_ERR_ARG;
    }
    if (!valid_k_n(k, n)) {
        return SHAMIR_ERR_RANGE;
    }

    for (;;) {
        bool has_256 = false;

        for (int x = 1; x <= n; x++) {
            uint16_t y = evaluate_polynomial(coefficients, k, (uint16_t)x);
            if (y == 256U) {
                has_256 = true;
                break;
            }
            shadow_bytes[x - 1] = (uint8_t)y;
        }

        if (!has_256) {
            return SHAMIR_OK;
        }
        if (!decrement_first_nonzero(coefficients, k)) {
            return SHAMIR_ERR_INTERPOLATION;
        }
    }
}

ShamirError shamir_split(const uint8_t *secret, size_t secret_len,
                         int k, int n, uint16_t seed,
                         uint8_t *shadows, size_t shadow_len) {
    if (secret_len > 0 && (secret == NULL || shadows == NULL)) {
        return SHAMIR_ERR_ARG;
    }
    if (!valid_k_n(k, n)) {
        return SHAMIR_ERR_RANGE;
    }
    if (shadow_len != shamir_shadow_byte_count(secret_len, k)) {
        return SHAMIR_ERR_BUFFER;
    }

    PermutationTable table;
    permutation_table_set_seed(&table, seed);

    for (size_t block = 0; block < shadow_len; block++) {
        uint16_t coefficients[SHAMIR_MAX_K] = {0};
        uint8_t block_outputs[SHAMIR_MAX_N];

        for (int coeff = 0; coeff < k; coeff++) {
            size_t index = block * (size_t)k + (size_t)coeff;
            if (index < secret_len) {
                uint8_t random_value = permutation_table_next_char(&table);
                coefficients[coeff] = (uint16_t)(secret[index] ^ random_value);
            }
        }

        ShamirError err = shamir_evaluate_block(coefficients, k, n, block_outputs);
        if (err != SHAMIR_OK) {
            return err;
        }

        for (int shadow = 0; shadow < n; shadow++) {
            shadows[(size_t)shadow * shadow_len + block] = block_outputs[shadow];
        }
    }

    return SHAMIR_OK;
}

ShamirError shamir_recover(const uint8_t *shadows, const uint16_t *shadow_indices,
                           int k, size_t shadow_len, uint16_t seed,
                           uint8_t *secret_out, size_t secret_len) {
    if (secret_len > 0 && (shadows == NULL || shadow_indices == NULL || secret_out == NULL)) {
        return SHAMIR_ERR_ARG;
    }
    if (!valid_k_n(k, k)) {
        return SHAMIR_ERR_RANGE;
    }
    if (shadow_len != shamir_shadow_byte_count(secret_len, k)) {
        return SHAMIR_ERR_BUFFER;
    }

    PermutationTable table;
    permutation_table_set_seed(&table, seed);

    for (size_t block = 0; block < shadow_len; block++) {
        uint16_t x_values[SHAMIR_MAX_K];
        uint16_t y_values[SHAMIR_MAX_K];
        uint16_t coefficients[SHAMIR_MAX_K];

        for (int i = 0; i < k; i++) {
            if (shadow_indices[i] == 0 || shadow_indices[i] >= GF257_MODULUS) {
                return SHAMIR_ERR_RANGE;
            }
            x_values[i] = shadow_indices[i];
            y_values[i] = shadows[(size_t)i * shadow_len + block];
        }

        if (!lagrange_interpolate_coefficients(x_values, y_values, (size_t)k, coefficients)) {
            return SHAMIR_ERR_INTERPOLATION;
        }

        for (int coeff = 0; coeff < k; coeff++) {
            size_t index = block * (size_t)k + (size_t)coeff;
            if (index < secret_len) {
                uint8_t random_value = permutation_table_next_char(&table);
                secret_out[index] = (uint8_t)coefficients[coeff] ^ random_value;
            }
        }
    }

    return SHAMIR_OK;
}
