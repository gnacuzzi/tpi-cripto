#include "cli.h"

#include <stdio.h>

int main(int argc, char **argv) {
    CliConfig config;

    if (cli_parse(argc, argv, &config) != 0) {
        return 1;
    }

    (void)config;
    return 0;
}
