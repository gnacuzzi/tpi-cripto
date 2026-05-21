#include "gf257.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

static void print_ok(const char *test_name) {
    printf("[OK] %s\n", test_name);
}

static void test_normalize(void) {
    assert(gf257_normalize(0) == 0);
    assert(gf257_normalize(256) == 256);
    assert(gf257_normalize(257) == 0);
    assert(gf257_normalize(258) == 1);
    assert(gf257_normalize(-1) == 256);
    assert(gf257_normalize(-258) == 256);
    assert(gf257_normalize(257000000000LL + 42) == 42);
    assert(gf257_normalize(-257000000000LL - 1) == 256);
    print_ok("normalize");
}

static void test_add_sub_mul(void) {
    assert(gf257_add(200, 100) == 43);
    assert(gf257_add(256, 1) == 0);

    assert(gf257_sub(3, 5) == 255);
    assert(gf257_sub(0, 1) == 256);
    assert(gf257_sub(5, 3) == 2);

    assert(gf257_mul(16, 16) == 256);
    assert(gf257_mul(256, 256) == 1);
    assert(gf257_mul(0, 123) == 0);
    print_ok("add/sub/mul");
}

static void test_closure(void) {
    for (uint16_t a = 0; a < GF257_MODULUS; a++) {
        for (uint16_t b = 0; b < GF257_MODULUS; b++) {
            assert(gf257_add(a, b) < GF257_MODULUS);
            assert(gf257_sub(a, b) < GF257_MODULUS);
            assert(gf257_mul(a, b) < GF257_MODULUS);
        }
    }
    print_ok("closure");
}

static void test_identities(void) {
    for (uint16_t value = 0; value < GF257_MODULUS; value++) {
        assert(gf257_add(value, 0) == value);
        assert(gf257_sub(value, 0) == value);
        assert(gf257_sub(value, value) == 0);
        assert(gf257_mul(value, 0) == 0);
        assert(gf257_mul(value, 1) == value);
    }
    print_ok("identities");
}

static void test_pow(void) {
    assert(gf257_pow(7, 0) == 1);
    assert(gf257_pow(7, 1) == 7);
    assert(gf257_pow(2, 8) == 256);
    assert(gf257_pow(256, 2) == 1);

    for (uint16_t value = 1; value < GF257_MODULUS; value++) {
        assert(gf257_pow(value, 256) == 1);
    }
    print_ok("pow");
}

static void test_inverse(void) {
    uint16_t inverse;

    assert(!gf257_inverse(0, &inverse));
    assert(!gf257_inverse(1, 0));

    for (uint16_t value = 1; value < GF257_MODULUS; value++) {
        assert(gf257_inverse(value, &inverse));
        assert(gf257_mul(value, inverse) == 1);
    }
    print_ok("inverse");
}

static void test_div(void) {
    uint16_t result;

    assert(!gf257_div(10, 0, &result));
    assert(!gf257_div(10, 2, 0));

    assert(gf257_div(10, 2, &result));
    assert(result == 5);

    assert(gf257_div(1, 256, &result));
    assert(result == 256);

    assert(gf257_div(42, 7, &result));
    assert(result == 6);

    for (uint16_t numerator = 0; numerator < GF257_MODULUS; numerator++) {
        for (uint16_t denominator = 1; denominator < GF257_MODULUS; denominator++) {
            assert(gf257_div(numerator, denominator, &result));
            assert(gf257_mul(result, denominator) == numerator);
        }
    }
    print_ok("div");
}

int main(void) {
    test_normalize();
    test_add_sub_mul();
    test_closure();
    test_identities();
    test_pow();
    test_inverse();
    test_div();

    printf("gf257 tests passed\n");
    return 0;
}
