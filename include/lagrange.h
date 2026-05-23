#ifndef LAGRANGE_H
#define LAGRANGE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define LAGRANGE_MAX_K 10u

bool lagrange_interpolate_coefficients(const uint16_t *x_values,
                                       const uint16_t *y_values,
                                       size_t k,
                                       uint16_t *coefficients_out);

#endif
