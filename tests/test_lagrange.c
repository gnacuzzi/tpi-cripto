#include "gf257.h"
#include "lagrange.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static void print_ok(const char *test_name) {
    printf("[OK] %s\n", test_name);
}

static uint16_t eval_known_poly(uint16_t x) {
    /* 50 + 51x + 55x^2 mod 257 */
    uint16_t y = 50;
    y = gf257_add(y, gf257_mul(51, x));
    y = gf257_add(y, gf257_mul(55, gf257_mul(x, x)));
    return y;
}

static void test_recovers_coefficients(void) {
    uint16_t x_values[] = {1, 2, 5};
    uint16_t y_values[] = {
        eval_known_poly(1),
        eval_known_poly(2),
        eval_known_poly(5),
    };
    uint16_t coefficients[3];

    assert(lagrange_interpolate_coefficients(x_values, y_values, 3, coefficients));
    assert(coefficients[0] == 50);
    assert(coefficients[1] == 51);
    assert(coefficients[2] == 55);

    print_ok("recovers coefficients");
}

static void test_single_point_recovers_constant(void) {
    uint16_t x_values[] = {9};
    uint16_t y_values[] = {123};
    uint16_t coefficients[1];

    assert(lagrange_interpolate_coefficients(x_values, y_values, 1, coefficients));
    assert(coefficients[0] == 123);

    print_ok("single point recovers constant");
}

static void test_rejects_duplicate_x(void) {
    uint16_t x_values[] = {1, 1, 3};
    uint16_t y_values[] = {10, 20, 30};
    uint16_t coefficients[3];

    assert(!lagrange_interpolate_coefficients(x_values, y_values, 3, coefficients));

    print_ok("rejects duplicate x");
}

static void test_rejects_values_outside_field(void) {
    uint16_t x_values[] = {1, 2, 257};
    uint16_t y_values[] = {10, 20, 30};
    uint16_t coefficients[3];

    assert(!lagrange_interpolate_coefficients(x_values, y_values, 3, coefficients));

    x_values[2] = 3;
    y_values[2] = 257;

    assert(!lagrange_interpolate_coefficients(x_values, y_values, 3, coefficients));

    print_ok("rejects values outside field");
}

int main(void) {
    test_recovers_coefficients();
    test_single_point_recovers_constant();
    test_rejects_duplicate_x();
    test_rejects_values_outside_field();

    printf("lagrange tests passed\n");
    return 0;
}
