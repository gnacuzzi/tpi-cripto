#include "lagrange.h"

#include "gf257.h"

#include <string.h>

static bool valid_inputs(const uint16_t *x_values, const uint16_t *y_values,
                         size_t k, const uint16_t *out) {
    return x_values != NULL && y_values != NULL && out != NULL
        && k > 0 && k <= LAGRANGE_MAX_K;
}

static bool x_values_are_unique(const uint16_t *x_values, size_t k) {
    for (size_t i = 0; i < k; i++) {
        if (x_values[i] >= GF257_MODULUS) {
            return false;
        }
        for (size_t j = i + 1; j < k; j++) {
            if (x_values[i] == x_values[j]) {
                return false;
            }
        }
    }
    return true;
}

static bool lagrange_l0(size_t k, const uint16_t *x_values, size_t i, uint16_t *out) {
    uint16_t ans = 1;

    for (size_t j = 0; j < k; j++) {
        if (j != i) {
            uint16_t factor;
            if (!gf257_div(gf257_sub(0, x_values[j]),
                           gf257_sub(x_values[i], x_values[j]),
                           &factor)) {
                return false;
            }
            ans = gf257_mul(ans, factor);
        }
    }

    *out = ans;
    return true;
}

static bool lagrange_p0(size_t k, const uint16_t *x_values,
                        const uint16_t *y_values, uint16_t *out) {
    uint16_t ans = 0;

    for (size_t i = 0; i < k; i++) {
        uint16_t l0;
        if (!lagrange_l0(k, x_values, i, &l0)) {
            return false;
        }
        ans = gf257_add(ans, gf257_mul(l0, y_values[i]));
    }

    *out = ans;
    return true;
}

static bool decypher_lagrange(size_t k, const uint16_t *x_values,
                              uint16_t *y_values, uint16_t *coefficients_out) {
    uint16_t a0;

    if (k == 1) {
        coefficients_out[0] = y_values[0];
        return true;
    }

    if (!lagrange_p0(k, x_values, y_values, &a0)) {
        return false;
    }
    coefficients_out[0] = a0;

    for (size_t i = 0; i < k - 1U; i++) {
        if (!gf257_div(gf257_sub(y_values[i], a0), x_values[i], &y_values[i])) {
            return false;
        }
    }

    return decypher_lagrange(k - 1U, x_values, y_values, coefficients_out + 1);
}

bool lagrange_interpolate_coefficients(const uint16_t *x_values,
                                       const uint16_t *y_values,
                                       size_t k,
                                       uint16_t *coefficients_out) {
    if (!valid_inputs(x_values, y_values, k, coefficients_out)
        || !x_values_are_unique(x_values, k)) {
        return false;
    }

    for (size_t i = 0; i < k; i++) {
        if (y_values[i] >= GF257_MODULUS) {
            return false;
        }
    }

    uint16_t reduced_y[LAGRANGE_MAX_K];
    memcpy(reduced_y, y_values, k * sizeof(reduced_y[0]));
    memset(coefficients_out, 0, k * sizeof(coefficients_out[0]));

    return decypher_lagrange(k, x_values, reduced_y, coefficients_out);
}

