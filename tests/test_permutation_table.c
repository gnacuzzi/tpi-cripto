#include "permutation_table.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

static void print_ok(const char *test_name) {
    printf("[OK] %s\n", test_name);
}

static void test_seed_10_sequence(void) {
    static const uint8_t expected[] = {
        186, 114, 65,  105, 15,  172, 62,  94,  209, 91,
        94,  167, 219, 228, 183, 105, 73,  89,  234, 10,
        2,   235, 112, 29,  8,   176, 236, 46,  246, 48,
        204, 175, 111, 253, 208, 35,  55,  171, 244, 100,
        55,  66,  101, 176, 76,  53,  121, 74,  96,  42,
    };
    PermutationTable table;

    permutation_table_set_seed(&table, 10);

    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
        assert(permutation_table_next_char(&table) == expected[i]);
    }

    print_ok("seed 10 sequence");
}

static void test_fill_matches_next_char(void) {
    uint8_t from_fill[32];
    uint8_t from_next[32];
    PermutationTable fill_table;
    PermutationTable next_table;

    permutation_table_set_seed(&fill_table, 641);
    permutation_table_set_seed(&next_table, 641);

    permutation_table_fill(&fill_table, from_fill, sizeof(from_fill));

    for (size_t i = 0; i < sizeof(from_next); i++) {
        from_next[i] = permutation_table_next_char(&next_table);
    }

    for (size_t i = 0; i < sizeof(from_fill); i++) {
        assert(from_fill[i] == from_next[i]);
    }

    print_ok("fill matches next_char");
}

static void test_reseed_restarts_sequence(void) {
    PermutationTable table;
    uint8_t first;
    uint8_t second;

    permutation_table_set_seed(&table, 1234);
    first = permutation_table_next_char(&table);

    permutation_table_set_seed(&table, 1234);
    second = permutation_table_next_char(&table);

    assert(first == second);

    print_ok("reseed restarts sequence");
}

int main(void) {
    test_seed_10_sequence();
    test_fill_matches_next_char();
    test_reseed_restarts_sequence();

    printf("permutation_table tests passed\n");
    return 0;
}
