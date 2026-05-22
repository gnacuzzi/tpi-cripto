#include "bmp.h"
#include "stego.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_ok(const char *test_name) {
    printf("[OK] %s\n", test_name);
}

static void assert_capacity(size_t secret_pixels, int k, size_t expected_shadow,
                            size_t expected_required) {
    assert(stego_shadow_byte_count(secret_pixels, k) == expected_shadow);
    assert(stego_required_carrier_pixels(secret_pixels, k) == expected_required);
}

static BmpImage make_carrier(int32_t width, int32_t height) {
    BmpImage carrier;
    memset(&carrier, 0, sizeof(carrier));
    carrier.width = width;
    carrier.height = height;
    size_t count = bmp_pixel_count(&carrier);
    carrier.pixels = calloc(count, 1);
    assert(carrier.pixels != NULL);
    return carrier;
}

static void test_capacity_k8(void) {
    assert(stego_shadow_byte_count(64, 8) == 8U);
    assert(stego_required_carrier_pixels(64, 8) == 64U);
    assert(stego_shadow_byte_count(65, 8) == 9U);
    assert(stego_required_carrier_pixels(65, 8) == 72U);
    print_ok("capacity k=8");
}

static void test_capacity_k2(void) {
    assert(stego_shadow_byte_count(64, 2) == 32U);
    assert(stego_required_carrier_pixels(64, 2) == 256U);
    print_ok("capacity k=2");
}

static void test_capacity_odd_k(void) {
    assert_capacity(63, 3, 21, 168);
    assert_capacity(63, 5, 13, 104);
    assert_capacity(63, 7, 9, 72);
    assert_capacity(63, 9, 7, 56);
    print_ok("capacity odd k (3,5,7,9)");
}

static void test_capacity_remainder(void) {
    assert_capacity(65, 3, 22, 176);
    assert_capacity(65, 5, 13, 104);
    assert_capacity(65, 7, 10, 80);
    assert_capacity(65, 9, 8, 64);
    assert_capacity(65, 10, 7, 56);
    assert_capacity(100, 7, 15, 120);
    print_ok("capacity with remainder (odd k and k=10)");
}

static void test_embed_extract_shadow_for_k(void) {
    enum { SECRET_PIXELS = 50, K = 5 };
    size_t shadow_len = stego_shadow_byte_count(SECRET_PIXELS, K);
    size_t required = stego_required_carrier_pixels(SECRET_PIXELS, K);
    assert(shadow_len == 10U);
    assert(required == 80U);

    uint8_t shadow[10];
    for (size_t i = 0; i < shadow_len; i++) {
        shadow[i] = (uint8_t)(0xA0U + i);
    }

    uint8_t pixels[80];
    for (size_t i = 0; i < required; i++) {
        pixels[i] = (uint8_t)(0x40U + (uint8_t)(i % 128U));
    }

    assert(lsb_embed_pixels(pixels, required, shadow, shadow_len) == STEGO_OK);

    uint8_t out[10];
    assert(lsb_extract_pixels(pixels, required, out, shadow_len) == STEGO_OK);
    assert(memcmp(out, shadow, shadow_len) == 0);

    print_ok("embed/extract shadow sized for k=5");
}

static void test_d1_example(void) {
    static const uint8_t before[] = {
        0xED, 0xA4, 0x45, 0x36, 0x3A, 0x3A, 0x3A, 0x39
    };
    static const uint8_t expected[] = {
        0xED, 0xA5, 0x44, 0x37, 0x3A, 0x3A, 0x3A, 0x39
    };

    uint8_t pixels[8];
    memcpy(pixels, before, sizeof(before));

    assert(lsb_embed_pixels(pixels, 8, (const uint8_t[]){0xD1}, 1) == STEGO_OK);
    assert(memcmp(pixels, expected, sizeof(expected)) == 0);

    uint8_t extracted = 0;
    assert(lsb_extract_pixels(pixels, 8, &extracted, 1) == STEGO_OK);
    assert(extracted == 0xD1);

    print_ok("0xD1 enunciado example");
}

static void test_embed_extract_identity(void) {
    static const uint8_t shadow[] = {
        0x00, 0xFF, 0xD1, 0x42, 0x81, 0x3C, 0xA5, 0x5A
    };
    uint8_t pixels[sizeof(shadow) * 8U];
    for (size_t i = 0; i < sizeof(pixels); i++) {
        pixels[i] = (uint8_t)(0x80U | (uint8_t)(i & 0x7FU));
    }

    assert(lsb_embed_pixels(pixels, sizeof(pixels), shadow, sizeof(shadow)) == STEGO_OK);

    uint8_t out[sizeof(shadow)];
    assert(lsb_extract_pixels(pixels, sizeof(pixels), out, sizeof(out)) == STEGO_OK);
    assert(memcmp(out, shadow, sizeof(shadow)) == 0);

    print_ok("embed/extract identity");
}

static void test_bmp_embed_extract(void) {
    static const uint8_t pixel_values[] = {
        0xED, 0xA4, 0x45, 0x36, 0x3A, 0x3A, 0x3A, 0x39,
        0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80
    };

    BmpImage img;
    memset(&img, 0, sizeof(img));
    img.width = 4;
    img.height = 4;
    img.pixel_offset = 54;
    img.header_size = 54;
    img.header = calloc(54, 1);
    img.pixels = malloc(sizeof(pixel_values));
    assert(img.header != NULL && img.pixels != NULL);
    memcpy(img.pixels, pixel_values, sizeof(pixel_values));

    static const uint8_t shadow[] = {0xD1, 0x42};
    assert(lsb_embed(&img, shadow, sizeof(shadow)) == STEGO_OK);

    uint8_t extracted[sizeof(shadow)];
    assert(lsb_extract(&img, extracted, sizeof(extracted)) == STEGO_OK);
    assert(memcmp(extracted, shadow, sizeof(shadow)) == 0);

    bmp_free(&img);
    print_ok("BmpImage embed/extract");
}

static void test_capacity_error(void) {
    uint8_t pixels[7] = {0};
    assert(lsb_embed_pixels(pixels, 7, (const uint8_t[]){0x01}, 1) == STEGO_ERR_CAPACITY);
    print_ok("capacity error");
}

static void test_validate_distribute_k8(void) {
    BmpImage carriers[3];
    for (size_t i = 0; i < 3; i++) {
        memset(&carriers[i], 0, sizeof(carriers[i]));
        carriers[i].width = 4;
        carriers[i].height = 4;
        carriers[i].pixels = calloc(16, 1);
        assert(carriers[i].pixels != NULL);
    }

    assert(stego_validate_carriers_distribute(carriers, 3, 4, 4, 8) == STEGO_OK);

    carriers[1].width = 5;
    assert(stego_validate_carriers_distribute(carriers, 3, 4, 4, 8) == STEGO_ERR_DIMENSIONS);
    carriers[1].width = 4;

    assert(stego_validate_carriers_distribute(carriers, 3, 8, 4, 8) == STEGO_ERR_DIMENSIONS);

    for (size_t i = 0; i < 3; i++) {
        bmp_free(&carriers[i]);
    }
    print_ok("validate distribute k=8");
}

static void test_validate_distribute_k3(void) {
    BmpImage carrier = make_carrier(12, 8);

    assert(stego_validate_carriers_distribute(&carrier, 1, 6, 6, 3) == STEGO_OK);

    bmp_free(&carrier);
    carrier = make_carrier(6, 6);
    assert(stego_validate_carriers_distribute(&carrier, 1, 6, 6, 3) == STEGO_ERR_CARRIER_SIZE);

    bmp_free(&carrier);
    print_ok("validate distribute k=3");
}

static void test_validate_distribute_k7_smaller_carrier(void) {
    BmpImage carrier = make_carrier(8, 7);

    assert(stego_validate_carriers_distribute(&carrier, 1, 7, 7, 7) == STEGO_OK);

    bmp_free(&carrier);
    carrier = make_carrier(7, 7);
    assert(stego_validate_carriers_distribute(&carrier, 1, 7, 7, 7) == STEGO_ERR_CARRIER_SIZE);

    bmp_free(&carrier);
    print_ok("validate distribute k=7");
}

static void test_validate_distribute_k9_minimal(void) {
    BmpImage carrier = make_carrier(9, 8);

    assert(stego_validate_carriers_distribute(&carrier, 1, 9, 9, 9) == STEGO_OK);

    bmp_free(&carrier);
    carrier = make_carrier(8, 8);
    assert(stego_validate_carriers_distribute(&carrier, 1, 9, 9, 9) == STEGO_ERR_CARRIER_SIZE);

    bmp_free(&carrier);
    print_ok("validate distribute k=9 minimal carrier");
}

static void test_validate_distribute_too_small(void) {
    BmpImage carrier;
    memset(&carrier, 0, sizeof(carrier));
    carrier.width = 4;
    carrier.height = 4;
    carrier.pixels = calloc(16, 1);
    assert(carrier.pixels != NULL);

    assert(stego_validate_carriers_distribute(&carrier, 1, 4, 4, 2) == STEGO_ERR_CARRIER_SIZE);
    bmp_free(&carrier);
    print_ok("validate carrier too small");
}

static void test_validate_recover(void) {
    BmpImage carriers[2];
    for (size_t i = 0; i < 2; i++) {
        memset(&carriers[i], 0, sizeof(carriers[i]));
        carriers[i].width = 4;
        carriers[i].height = 4;
        carriers[i].pixels = calloc(16, 1);
        assert(carriers[i].pixels != NULL);
    }

    assert(stego_validate_carriers_recover(carriers, 2, 8) == STEGO_ERR_COUNT);
    assert(stego_validate_carriers_recover(carriers, 2, 2) == STEGO_OK);

    carriers[1].height = 5;
    assert(stego_validate_carriers_recover(carriers, 2, 2) == STEGO_ERR_DIMENSIONS);

    for (size_t i = 0; i < 2; i++) {
        bmp_free(&carriers[i]);
    }
    print_ok("validate recover");
}

static void test_validate_recover_odd_k(void) {
    BmpImage carriers[5];
    for (size_t i = 0; i < 5; i++) {
        carriers[i] = make_carrier(16, 16);
    }

    assert(stego_validate_carriers_recover(carriers, 5, 3) == STEGO_OK);
    assert(stego_validate_carriers_recover(carriers, 5, 5) == STEGO_OK);
    assert(stego_validate_carriers_recover(carriers, 4, 5) == STEGO_ERR_COUNT);
    assert(stego_validate_carriers_recover(carriers, 3, 5) == STEGO_ERR_COUNT);

    for (size_t i = 0; i < 5; i++) {
        bmp_free(&carriers[i]);
    }
    print_ok("validate recover odd k (3,5)");
}

int main(void) {
    test_capacity_k8();
    test_capacity_k2();
    test_capacity_odd_k();
    test_capacity_remainder();
    test_embed_extract_shadow_for_k();
    test_d1_example();
    test_embed_extract_identity();
    test_bmp_embed_extract();
    test_capacity_error();
    test_validate_distribute_k8();
    test_validate_distribute_k3();
    test_validate_distribute_k7_smaller_carrier();
    test_validate_distribute_k9_minimal();
    test_validate_distribute_too_small();
    test_validate_recover();
    test_validate_recover_odd_k();
    printf("lsb tests passed\n");
    return 0;
}
