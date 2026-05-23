#include "shamir.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void print_ok(const char *test_name) {
    printf("[OK] %s\n", test_name);
}

static void test_evaluate_block_adjusts_256(void) {
    uint16_t coefficients[] = {255, 1};
    uint8_t outputs[3];

    assert(shamir_evaluate_block(coefficients, 2, 3, outputs) == SHAMIR_OK);
    assert(coefficients[0] == 252);
    assert(outputs[0] == 253);
    assert(outputs[1] == 254);
    assert(outputs[2] == 255);

    print_ok("evaluate block adjusts 256");
}

static void test_split_recover_exact_for_subset(void) {
    static const uint8_t secret[] = {
        1, 2, 3, 4, 5, 6, 7, 8, 9
    };
    enum { K = 3, N = 5 };
    const uint16_t seed = 10;
    size_t shadow_len = shamir_shadow_byte_count(sizeof(secret), K);
    uint8_t shadows[(size_t)N * 3U];
    uint8_t selected[(size_t)K * 3U];
    uint16_t indices[] = {1, 3, 5};
    uint8_t recovered[sizeof(secret)];

    assert(shadow_len == 3U);
    assert(shamir_split(secret, sizeof(secret), K, N, seed, shadows, shadow_len) == SHAMIR_OK);

    memcpy(selected + 0U * shadow_len, shadows + 0U * shadow_len, shadow_len);
    memcpy(selected + 1U * shadow_len, shadows + 2U * shadow_len, shadow_len);
    memcpy(selected + 2U * shadow_len, shadows + 4U * shadow_len, shadow_len);

    assert(shamir_recover(selected, indices, K, shadow_len, seed,
                          recovered, sizeof(recovered)) == SHAMIR_OK);
    assert(memcmp(recovered, secret, sizeof(secret)) == 0);

    print_ok("split/recover exact subset");
}

static void test_split_recover_with_padding(void) {
    static const uint8_t secret[] = {11, 22, 33, 44, 55};
    enum { K = 4, N = 6 };
    const uint16_t seed = 641;
    size_t shadow_len = shamir_shadow_byte_count(sizeof(secret), K);
    uint8_t shadows[(size_t)N * 2U];
    uint8_t selected[(size_t)K * 2U];
    uint16_t indices[] = {2, 3, 5, 6};
    uint8_t recovered[sizeof(secret)];

    assert(shadow_len == 2U);
    assert(shamir_split(secret, sizeof(secret), K, N, seed, shadows, shadow_len) == SHAMIR_OK);

    memcpy(selected + 0U * shadow_len, shadows + 1U * shadow_len, shadow_len);
    memcpy(selected + 1U * shadow_len, shadows + 2U * shadow_len, shadow_len);
    memcpy(selected + 2U * shadow_len, shadows + 4U * shadow_len, shadow_len);
    memcpy(selected + 3U * shadow_len, shadows + 5U * shadow_len, shadow_len);

    assert(shamir_recover(selected, indices, K, shadow_len, seed,
                          recovered, sizeof(recovered)) == SHAMIR_OK);
    assert(memcmp(recovered, secret, sizeof(secret)) == 0);

    print_ok("split/recover with padding");
}

static void test_recover_rejects_duplicate_indices(void) {
    static const uint8_t shadows[] = {1, 2, 3, 4};
    uint16_t indices[] = {1, 1};
    uint8_t recovered[4];

    assert(shamir_recover(shadows, indices, 2, 2, 10, recovered, sizeof(recovered))
           == SHAMIR_ERR_INTERPOLATION);

    print_ok("recover rejects duplicate indices");
}

int main(void) {
    test_evaluate_block_adjusts_256();
    test_split_recover_exact_for_subset();
    test_split_recover_with_padding();
    test_recover_rejects_duplicate_indices();

    printf("shamir tests passed\n");
    return 0;
}
