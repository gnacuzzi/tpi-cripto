#ifndef CLI_H
#define CLI_H

#include <stdbool.h>
#include <stdio.h>

typedef enum {
    CLI_MODE_NONE = 0,
    CLI_MODE_DISTRIBUTE,
    CLI_MODE_RECOVER
} CliMode;

typedef struct {
    CliMode mode;
    const char *secret_path;
    int k;
    int n;
    const char *dir;
} CliConfig;

void cli_print_usage(FILE *stream);

/* Returns 0 on success, non-zero on parse/validation error. */
int cli_parse(int argc, char **argv, CliConfig *config);

#endif
