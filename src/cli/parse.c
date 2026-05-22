#include "cli.h"

#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define K_MIN 2
#define K_MAX 10

static struct option long_options[] = {
    {"d", no_argument, NULL, 'd'},
    {"r", no_argument, NULL, 'r'},
    {"secret", required_argument, NULL, 's'},
    {"k", required_argument, NULL, 'k'},
    {"n", required_argument, NULL, 'n'},
    {"dir", required_argument, NULL, 'D'},
    {NULL, 0, NULL, 0},
};

static void normalize_argv(int argc, char **argv, char **normalized) {
    static const struct {
        const char *from;
        const char *to;
    } rewrites[] = {
        {"-secret", "--secret"},
        {"-dir", "--dir"},
    };

    for (int i = 0; i < argc; i++) {
        normalized[i] = argv[i];
        for (size_t r = 0; r < sizeof(rewrites) / sizeof(rewrites[0]); r++) {
            if (strcmp(argv[i], rewrites[r].from) == 0) {
                normalized[i] = (char *)rewrites[r].to;
                break;
            }
        }
    }
    normalized[argc] = NULL;
}

static void print_error(const char *message) {
    fprintf(stderr, "error: %s\n", message);
}

static bool has_bmp_extension(const char *name) {
    const char *dot = strrchr(name, '.');
    return dot != NULL && strcmp(dot, ".bmp") == 0;
}

static bool parse_int(const char *text, int *value) {
    char *end = NULL;
    long parsed = strtol(text, &end, 10);

    if (end == text || *end != '\0') {
        return false;
    }
    if (parsed < 0 || parsed > INT_MAX) {
        return false;
    }

    *value = (int)parsed;
    return true;
}

static bool path_exists(const char *path) {
    struct stat info;

    return stat(path, &info) == 0;
}

static bool is_directory_readable(const char *path) {
    struct stat info;

    if (stat(path, &info) != 0) {
        return false;
    }
    if (!S_ISDIR(info.st_mode)) {
        return false;
    }

    return access(path, R_OK | X_OK) == 0;
}

static int count_bmp_files(const char *dir_path) {
    DIR *dir = opendir(dir_path);
    int count = 0;

    if (dir == NULL) {
        print_error("cannot read directory");
        return -1;
    }

    errno = 0;
    struct dirent *entry;
    while((entry = readdir(dir))) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        if (has_bmp_extension(entry->d_name)) {
            count++;
        }
    }
    if (errno != 0) {
        closedir(dir);
        print_error("cannot read directory");
        return -1;
    }

    closedir(dir);
    return count;
}

void cli_print_usage(FILE *stream) {
    fprintf(stream,
            "usage: visualSSS -d|-r -secret <file.bmp> -k <2..10> [-n <int>] [-dir <path>]\n"
            "\n"
            "  -d              distribute secret into carrier BMP images\n"
            "  -r              recover secret from carrier BMP images\n"
            "  -secret <file>  input secret BMP (-d) or output secret BMP (-r)\n"
            "  -k <int>        threshold k for (k, n) scheme\n"
            "  -n <int>        total shares n (distribution only; default: BMP count in -dir)\n"
            "  -dir <path>     directory with carrier images (default: current directory)\n");
}

int cli_parse(int argc, char **argv, CliConfig *config) {
    int opt = 0;
    char *normalized[argc + 1];
    bool k_seen = false;
    bool n_seen = false;

    config->mode = CLI_MODE_NONE;
    config->secret_path = NULL;
    config->k = 0;
    config->n = 0;
    config->dir = NULL;

    if (argc <= 1) {
        print_error("missing arguments");
        cli_print_usage(stderr);
        return 1;
    }

    normalize_argv(argc, argv, normalized);

    opterr = 0;
    optind = 1;

    while ((opt = getopt_long(argc, normalized, ":drs:k:n:D:", long_options, NULL)) != -1) {
        switch (opt) {
        case 'd':
            if (config->mode != CLI_MODE_NONE) {
                print_error("mode already specified");
                cli_print_usage(stderr);
                return 1;
            }
            config->mode = CLI_MODE_DISTRIBUTE;
            break;

        case 'r':
            if (config->mode != CLI_MODE_NONE) {
                print_error("mode already specified");
                cli_print_usage(stderr);
                return 1;
            }
            config->mode = CLI_MODE_RECOVER;
            break;

        case 's':
            if (config->secret_path != NULL) {
                print_error("duplicate -secret");
                cli_print_usage(stderr);
                return 1;
            }
            config->secret_path = optarg;
            break;

        case 'k':
            if (k_seen) {
                print_error("duplicate -k");
                cli_print_usage(stderr);
                return 1;
            }
            k_seen = true;
            if (!parse_int(optarg, &config->k)) {
                print_error("invalid -k value");
                cli_print_usage(stderr);
                return 1;
            }
            break;

        case 'n':
            if (n_seen) {
                print_error("duplicate -n");
                cli_print_usage(stderr);
                return 1;
            }
            n_seen = true;
            if (!parse_int(optarg, &config->n)) {
                print_error("invalid -n value");
                cli_print_usage(stderr);
                return 1;
            }
            break;

        case 'D':
            if (config->dir != NULL) {
                print_error("duplicate -dir");
                cli_print_usage(stderr);
                return 1;
            }
            config->dir = optarg;
            break;

        case ':':
            print_error("missing option value");
            cli_print_usage(stderr);
            return 1;

        case '?':
        default:
            print_error("unknown argument");
            cli_print_usage(stderr);
            return 1;
        }
    }

    if (optind < argc) {
        print_error("unknown argument");
        cli_print_usage(stderr);
        return 1;
    }

    if (config->mode == CLI_MODE_NONE) {
        print_error("missing mode (-d or -r)");
        cli_print_usage(stderr);
        return 1;
    }

    if (!config->secret_path) {
        print_error("missing -secret");
        cli_print_usage(stderr);
        return 1;
    }

    if (!k_seen) {
        print_error("missing -k");
        cli_print_usage(stderr);
        return 1;
    }

    if (config->mode == CLI_MODE_RECOVER && config->n) {
        print_error("-n is only valid with -d");
        cli_print_usage(stderr);
        return 1;
    }

    if(!config->dir)
        config->dir = ".";

    if (!is_directory_readable(config->dir)) {
        print_error("directory is not readable");
        cli_print_usage(stderr);
        return 1;
    }

    if (config->k < K_MIN || config->k > K_MAX) {
        print_error("k must be between 2 and 10");
        cli_print_usage(stderr);
        return 1;
    }

    if (config->mode == CLI_MODE_DISTRIBUTE) {
        if (!path_exists(config->secret_path)) {
            print_error("secret file does not exist");
            cli_print_usage(stderr);
            return 1;
        }

        if (!config->n) {
            int bmp_count = count_bmp_files(config->dir);
            if (bmp_count < 0) {
                cli_print_usage(stderr);
                return 1;
            }
            config->n = bmp_count;
        }

        if (config->n < K_MIN) {
            print_error("n must be at least 2");
            cli_print_usage(stderr);
            return 1;
        }

        if (config->k > config->n) {
            print_error("k cannot be greater than n");
            cli_print_usage(stderr);
            return 1;
        }
    }

    return 0;
}
