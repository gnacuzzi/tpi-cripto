#include "permutation_table.h"

#define PERMUTATION_TABLE_MULTIPLIER 0x5DEECE66DLL
#define PERMUTATION_TABLE_ADDEND 0xBLL
#define PERMUTATION_TABLE_MASK ((1LL << 48) - 1)

void permutation_table_set_seed(PermutationTable *table, int64_t seed) {
    table->seed = (seed ^ PERMUTATION_TABLE_MULTIPLIER) & PERMUTATION_TABLE_MASK;
}

uint8_t permutation_table_next_char(PermutationTable *table) {
    table->seed =
        (table->seed * PERMUTATION_TABLE_MULTIPLIER + PERMUTATION_TABLE_ADDEND) &
        PERMUTATION_TABLE_MASK;

    return (uint8_t)(table->seed >> 40);
}

void permutation_table_fill(PermutationTable *table, uint8_t *buffer, size_t length) {
    for (size_t i = 0; i < length; i++) {
        buffer[i] = permutation_table_next_char(table);
    }
}
