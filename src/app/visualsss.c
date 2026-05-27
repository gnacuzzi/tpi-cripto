#include "app.h"

#include "bmp.h"
#include "shamir.h"
#include "stego.h"

#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

typedef struct {
    char **items;
    size_t count;
} PathList;

static int has_bmp_extension(const char *name) {
    const char *dot = strrchr(name, '.');
    return dot != NULL && strcmp(dot, ".bmp") == 0;
}

static int path_join(const char *dir, const char *name, char **out) {
    size_t dir_len = strlen(dir);
    size_t name_len = strlen(name);
    int needs_slash = dir_len > 0 && dir[dir_len - 1U] != '/';
    size_t len = dir_len + (size_t)needs_slash + name_len + 1U;

    char *path = malloc(len);
    if (path == NULL) {
        return 0;
    }

    snprintf(path, len, "%s%s%s", dir, needs_slash ? "/" : "", name);
    *out = path;
    return 1;
}

static int compare_strings(const void *a, const void *b) {
    const char *const *sa = a;
    const char *const *sb = b;
    return strcmp(*sa, *sb);
}

static void path_list_free(PathList *list) {
    if (list == NULL) {
        return;
    }
    for (size_t i = 0; i < list->count; i++) {
        free(list->items[i]);
    }
    free(list->items);
    list->items = NULL;
    list->count = 0;
}

static int path_list_push(PathList *list, char *path) {
    char **new_items = realloc(list->items, (list->count + 1U) * sizeof(list->items[0]));
    if (new_items == NULL) {
        return 0;
    }
    list->items = new_items;
    list->items[list->count] = path;
    list->count++;
    return 1;
}

static int same_path_text(const char *a, const char *b) {
    return a != NULL && b != NULL && strcmp(a, b) == 0;
}

static int same_existing_file(const char *a, const char *b) {
    struct stat stat_a;
    struct stat stat_b;

    if (a == NULL || b == NULL) {
        return 0;
    }
    if (stat(a, &stat_a) == 0 && stat(b, &stat_b) == 0) {
        return stat_a.st_dev == stat_b.st_dev && stat_a.st_ino == stat_b.st_ino;
    }

    return same_path_text(a, b);
}

static int list_bmp_paths(const char *dir_path, const char *exclude_path, PathList *out) {
    memset(out, 0, sizeof(*out));

    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        fprintf(stderr, "error: cannot read directory\n");
        return 0;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        if (!has_bmp_extension(entry->d_name)) {
            continue;
        }

        char *path = NULL;
        if (!path_join(dir_path, entry->d_name, &path)) {
            closedir(dir);
            path_list_free(out);
            fprintf(stderr, "error: out of memory\n");
            return 0;
        }

        if (same_existing_file(path, exclude_path)) {
            free(path);
            continue;
        }

        if (!path_list_push(out, path)) {
            free(path);
            closedir(dir);
            path_list_free(out);
            fprintf(stderr, "error: out of memory\n");
            return 0;
        }
    }

    closedir(dir);
    qsort(out->items, out->count, sizeof(out->items[0]), compare_strings);
    return 1;
}

static void free_images(BmpImage *images, size_t count) {
    if (images == NULL) {
        return;
    }
    for (size_t i = 0; i < count; i++) {
        bmp_free(&images[i]);
    }
    free(images);
}

static int read_images(const PathList *paths, size_t count, BmpImage **out) {
    BmpImage *images = calloc(count, sizeof(images[0]));
    if (images == NULL) {
        fprintf(stderr, "error: out of memory\n");
        return 0;
    }

    for (size_t i = 0; i < count; i++) {
        BmpError err = bmp_read(paths->items[i], &images[i]);
        if (err != BMP_OK) {
            fprintf(stderr, "error: %s: %s\n", paths->items[i], bmp_error_string(err));
            free_images(images, count);
            return 0;
        }
    }

    *out = images;
    return 1;
}

static uint16_t make_seed(void) {
    const char *fixed_seed = getenv("VISUALSSS_SEED");
    if (fixed_seed != NULL && fixed_seed[0] != '\0') {
        char *end = NULL;
        errno = 0;
        unsigned long parsed = strtoul(fixed_seed, &end, 10);
        if (errno == 0 && end != fixed_seed && *end == '\0' && parsed <= USHRT_MAX) {
            return (uint16_t)parsed;
        }
    }

    uint32_t value = (uint32_t)time(NULL) ^ (uint32_t)getpid();
    value ^= value >> 16;
    return (uint16_t)(value & 0xFFFFU);
}

static uint32_t abs_height_u32(int32_t height) {
    return (uint32_t)(height < 0 ? -height : height);
}

static void write_u32_le(uint8_t *p, uint32_t value) {
    p[0] = (uint8_t)(value & 0xFFU);
    p[1] = (uint8_t)((value >> 8) & 0xFFU);
    p[2] = (uint8_t)((value >> 16) & 0xFFU);
    p[3] = (uint8_t)((value >> 24) & 0xFFU);
}

static int bmp_prepare_secret_output(BmpImage *img, uint8_t *secret_pixels,
                                     int32_t secret_width, int32_t secret_height) {
    if (img == NULL || img->header == NULL || img->header_size < 54U
        || secret_pixels == NULL || secret_width <= 0 || secret_height == 0) {
        return 0;
    }

    img->width = secret_width;
    img->height = secret_height;
    free(img->pixels);
    img->pixels = secret_pixels;

    write_u32_le(img->header + 18, (uint32_t)secret_width);
    write_u32_le(img->header + 22, (uint32_t)secret_height);

    size_t pixel_data_size = bmp_row_size(secret_width) * (size_t)abs_height_u32(secret_height);
    write_u32_le(img->header + 34, (uint32_t)pixel_data_size);
    bmp_set_seed(img, 0);
    bmp_set_shadow_index(img, 0);
    bmp_set_secret_dims(img, 0, 0);

    return 1;
}

static int write_images(const PathList *paths, const BmpImage *images, size_t count) {
    for (size_t i = 0; i < count; i++) {
        BmpError err = bmp_write(paths->items[i], &images[i]);
        if (err != BMP_OK) {
            fprintf(stderr, "error: %s: %s\n", paths->items[i], bmp_error_string(err));
            return 0;
        }
    }
    return 1;
}

static int run_distribute(const CliConfig *config) {
    BmpImage secret;
    BmpError bmp_err = bmp_read(config->secret_path, &secret);
    if (bmp_err != BMP_OK) {
        fprintf(stderr, "error: %s: %s\n", config->secret_path, bmp_error_string(bmp_err));
        return 1;
    }

    PathList paths;
    if (!list_bmp_paths(config->dir, config->secret_path, &paths)) {
        bmp_free(&secret);
        return 1;
    }

    if (paths.count < (size_t)config->n) {
        fprintf(stderr, "error: not enough carrier BMP files\n");
        path_list_free(&paths);
        bmp_free(&secret);
        return 1;
    }

    BmpImage *carriers = NULL;
    if (!read_images(&paths, (size_t)config->n, &carriers)) {
        path_list_free(&paths);
        bmp_free(&secret);
        return 1;
    }

    StegoError stego_err = stego_validate_carriers_distribute(
        carriers, (size_t)config->n, secret.width, secret.height, config->k);
    if (stego_err != STEGO_OK) {
        fprintf(stderr, "error: %s\n", stego_error_string(stego_err));
        free_images(carriers, (size_t)config->n);
        path_list_free(&paths);
        bmp_free(&secret);
        return 1;
    }

    size_t secret_len = bmp_pixel_count(&secret);
    size_t shadow_len = shamir_shadow_byte_count(secret_len, config->k);
    uint8_t *shadows = calloc((size_t)config->n * shadow_len, sizeof(shadows[0]));
    if (shadows == NULL) {
        fprintf(stderr, "error: out of memory\n");
        free_images(carriers, (size_t)config->n);
        path_list_free(&paths);
        bmp_free(&secret);
        return 1;
    }

    uint16_t seed = make_seed();
    ShamirError shamir_err = shamir_split(secret.pixels, secret_len, config->k,
                                          config->n, seed, shadows, shadow_len);
    if (shamir_err != SHAMIR_OK) {
        fprintf(stderr, "error: %s\n", shamir_error_string(shamir_err));
        free(shadows);
        free_images(carriers, (size_t)config->n);
        path_list_free(&paths);
        bmp_free(&secret);
        return 1;
    }

    for (int i = 0; i < config->n; i++) {
        uint8_t *shadow = shadows + (size_t)i * shadow_len;
        stego_err = lsb_embed(&carriers[i], shadow, shadow_len);
        if (stego_err != STEGO_OK) {
            fprintf(stderr, "error: %s\n", stego_error_string(stego_err));
            free(shadows);
            free_images(carriers, (size_t)config->n);
            path_list_free(&paths);
            bmp_free(&secret);
            return 1;
        }

        bmp_set_seed(&carriers[i], seed);
        bmp_set_shadow_index(&carriers[i], (uint16_t)(i + 1));
        if (config->k != 8) {
            bmp_set_secret_dims(&carriers[i], secret.width, secret.height);
        }
    }

    int ok = write_images(&paths, carriers, (size_t)config->n);

    free(shadows);
    free_images(carriers, (size_t)config->n);
    path_list_free(&paths);
    bmp_free(&secret);
    return ok ? 0 : 1;
}

static int valid_shadow_metadata(const BmpImage *img, int k) {
    uint16_t index = bmp_get_shadow_index(img);

    return k >= 2 && k <= 10 && index > 0 && index < 257U;
}

static int select_recovery_carriers(const BmpImage *images, size_t image_count, int k,
                                    size_t *selected, uint16_t *indices,
                                    uint16_t *seed_out) {
    size_t selected_count = 0;
    uint16_t seed = 0;
    int seed_seen = 0;

    for (size_t i = 0; i < image_count && selected_count < (size_t)k; i++) {
        if (!valid_shadow_metadata(&images[i], k)) {
            continue;
        }

        uint16_t index = bmp_get_shadow_index(&images[i]);
        int duplicate = 0;
        for (size_t j = 0; j < selected_count; j++) {
            if (indices[j] == index) {
                duplicate = 1;
                break;
            }
        }
        if (duplicate) {
            continue;
        }

        uint16_t current_seed = bmp_get_seed(&images[i]);
        if (!seed_seen) {
            seed = current_seed;
            seed_seen = 1;
        } else if (seed != current_seed) {
            continue;
        }

        selected[selected_count] = i;
        indices[selected_count] = index;
        selected_count++;
    }

    if (selected_count != (size_t)k) {
        return 0;
    }

    *seed_out = seed;
    return 1;
}

static int run_recover(const CliConfig *config) {
    PathList paths;
    if (!list_bmp_paths(config->dir, NULL, &paths)) {
        return 1;
    }

    if (paths.count < (size_t)config->k) {
        fprintf(stderr, "error: not enough carrier BMP files\n");
        path_list_free(&paths);
        return 1;
    }

    BmpImage *images = NULL;
    if (!read_images(&paths, paths.count, &images)) {
        path_list_free(&paths);
        return 1;
    }

    size_t selected[10];
    uint16_t indices[10];
    uint16_t seed = 0;
    if (!select_recovery_carriers(images, paths.count, config->k, selected, indices, &seed)) {
        fprintf(stderr, "error: could not find %d compatible shadow BMP files\n", config->k);
        free_images(images, paths.count);
        path_list_free(&paths);
        return 1;
    }

    BmpImage *template = &images[selected[0]];
    int32_t secret_width = template->width;
    int32_t secret_height = template->height;
    if (config->k != 8) {
        secret_width = bmp_get_secret_width(template);
        secret_height = bmp_get_secret_height(template);
        if (secret_width <= 0 || secret_height == 0) {
            fprintf(stderr, "error: missing secret dimensions in shadow metadata\n");
            free_images(images, paths.count);
            path_list_free(&paths);
            return 1;
        }
    }

    for (int i = 1; i < config->k; i++) {
        BmpImage *current = &images[selected[i]];
        if (current->width != template->width || current->height != template->height) {
            fprintf(stderr, "error: carrier dimensions mismatch\n");
            free_images(images, paths.count);
            path_list_free(&paths);
            return 1;
        }
        if (config->k != 8
            && (bmp_get_secret_width(current) != secret_width
                || bmp_get_secret_height(current) != secret_height)) {
            fprintf(stderr, "error: secret dimension metadata mismatch\n");
            free_images(images, paths.count);
            path_list_free(&paths);
            return 1;
        }
    }

    size_t secret_len = (size_t)secret_width * (size_t)abs_height_u32(secret_height);
    size_t shadow_len = shamir_shadow_byte_count(secret_len, config->k);

    uint8_t *selected_shadows = calloc((size_t)config->k * shadow_len, sizeof(selected_shadows[0]));
    uint8_t *secret_pixels = calloc(secret_len, sizeof(secret_pixels[0]));
    if (selected_shadows == NULL || secret_pixels == NULL) {
        fprintf(stderr, "error: out of memory\n");
        free(selected_shadows);
        free(secret_pixels);
        free_images(images, paths.count);
        path_list_free(&paths);
        return 1;
    }

    for (int i = 0; i < config->k; i++) {
        uint8_t *shadow = selected_shadows + (size_t)i * shadow_len;
        StegoError stego_err = lsb_extract(&images[selected[i]], shadow, shadow_len);
        if (stego_err != STEGO_OK) {
            fprintf(stderr, "error: %s\n", stego_error_string(stego_err));
            free(selected_shadows);
            free(secret_pixels);
            free_images(images, paths.count);
            path_list_free(&paths);
            return 1;
        }
    }

    ShamirError shamir_err = shamir_recover(selected_shadows, indices, config->k,
                                            shadow_len, seed, secret_pixels, secret_len);
    if (shamir_err != SHAMIR_OK) {
        fprintf(stderr, "error: %s\n", shamir_error_string(shamir_err));
        free(selected_shadows);
        free(secret_pixels);
        free_images(images, paths.count);
        path_list_free(&paths);
        return 1;
    }

    if (!bmp_prepare_secret_output(template, secret_pixels, secret_width, secret_height)) {
        fprintf(stderr, "error: invalid output image dimensions\n");
        free(selected_shadows);
        free(secret_pixels);
        free_images(images, paths.count);
        path_list_free(&paths);
        return 1;
    }
    secret_pixels = NULL;

    BmpError bmp_err = bmp_write(config->secret_path, template);
    if (bmp_err != BMP_OK) {
        fprintf(stderr, "error: %s: %s\n", config->secret_path, bmp_error_string(bmp_err));
        free(selected_shadows);
        free(secret_pixels);
        free_images(images, paths.count);
        path_list_free(&paths);
        return 1;
    }

    free(selected_shadows);
    free(secret_pixels);
    free_images(images, paths.count);
    path_list_free(&paths);
    return 0;
}

int app_run(const CliConfig *config) {
    if (config == NULL) {
        fprintf(stderr, "error: invalid configuration\n");
        return 1;
    }

    if (config->mode == CLI_MODE_DISTRIBUTE) {
        return run_distribute(config);
    }
    if (config->mode == CLI_MODE_RECOVER) {
        return run_recover(config);
    }

    fprintf(stderr, "error: invalid mode\n");
    return 1;
}
