#ifndef PERMUTATION_TABLE_H
#define PERMUTATION_TABLE_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    int64_t seed;
} PermutationTable;

void permutation_table_set_seed(PermutationTable *table, int64_t seed);
uint8_t permutation_table_next_char(PermutationTable *table);
void permutation_table_fill(PermutationTable *table, uint8_t *buffer, size_t length);

#endif
