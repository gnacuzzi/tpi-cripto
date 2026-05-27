#include "bmp.h"

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define PATH_BUF_SIZE 512U
#define CMD_BUF_SIZE 1024U

static void write_u16_le(uint8_t *p, uint16_t value) {
    p[0] = (uint8_t)(value & 0xFFU);
    p[1] = (uint8_t)((value >> 8) & 0xFFU);
}

static void write_u32_le(uint8_t *p, uint32_t value) {
    p[0] = (uint8_t)(value & 0xFFU);
    p[1] = (uint8_t)((value >> 8) & 0xFFU);
    p[2] = (uint8_t)((value >> 16) & 0xFFU);
    p[3] = (uint8_t)((value >> 24) & 0xFFU);
}

static void make_image(BmpImage *img, int32_t width, int32_t height,
                       const uint8_t *pixels) {
    memset(img, 0, sizeof(*img));

    size_t pixel_count = (size_t)width * (size_t)height;
    img->header = calloc(54U, 1);
    img->pixels = malloc(pixel_count);
    assert(img->header != NULL);
    assert(img->pixels != NULL);

    img->header_size = 54U;
    img->width = width;
    img->height = height;
    img->pixel_offset = 54U;

    img->header[0] = 'B';
    img->header[1] = 'M';
    write_u32_le(img->header + 10, 54U);
    write_u32_le(img->header + 14, 40U);
    write_u32_le(img->header + 18, (uint32_t)width);
    write_u32_le(img->header + 22, (uint32_t)height);
    write_u16_le(img->header + 26, 1U);
    write_u16_le(img->header + 28, 8U);
    write_u32_le(img->header + 30, 0U);

    memcpy(img->pixels, pixels, pixel_count);
}

static void write_fixture_bmp(const char *path, int32_t width, int32_t height,
                              const uint8_t *pixels) {
    BmpImage img;
    make_image(&img, width, height, pixels);
    assert(bmp_write(path, &img) == BMP_OK);
    bmp_free(&img);
}

static void run_command(const char *cmd) {
    int status = system(cmd);
    assert(status == 0);
}

static void checked_snprintf(char *buffer, size_t size, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int written = vsnprintf(buffer, size, format, args);
    va_end(args);

    assert(written >= 0 && (size_t)written < size);
}

static void fill_pixels(uint8_t *pixels, size_t count, uint8_t base) {
    for (size_t i = 0; i < count; i++) {
        pixels[i] = (uint8_t)(base + (uint8_t)(i % 113U));
    }
}

static void run_e2e_case(int k, int n, int32_t carrier_width, int32_t carrier_height) {
    char dir[PATH_BUF_SIZE];
    char secret_path[PATH_BUF_SIZE];
    char out_path[PATH_BUF_SIZE];
    char command[CMD_BUF_SIZE];
    uint8_t carrier_pixels[64];

    static const uint8_t secret_pixels[] = {
        186, 114, 65, 105, 15, 172, 62, 94
    };

    checked_snprintf(dir, sizeof(dir), "build/e2e_%ld_k%d", (long)getpid(), k);
    assert(mkdir(dir, 0700) == 0);

    checked_snprintf(secret_path, sizeof(secret_path), "%s/a_secret%d.bmp", dir, k);
    checked_snprintf(out_path, sizeof(out_path), "%s/out%d.bmp", dir, k);

    write_fixture_bmp(secret_path, 8, 1, secret_pixels);

    size_t carrier_count = (size_t)carrier_width * (size_t)carrier_height;
    assert(carrier_count <= sizeof(carrier_pixels));
    fill_pixels(carrier_pixels, carrier_count, (uint8_t)(10 + k));

    for (int i = 1; i <= n; i++) {
        char carrier_path[PATH_BUF_SIZE];
        checked_snprintf(carrier_path, sizeof(carrier_path), "%s/carrier%d.bmp", dir, i);
        write_fixture_bmp(carrier_path, carrier_width, carrier_height, carrier_pixels);
    }

    checked_snprintf(command, sizeof(command),
                     "VISUALSSS_SEED=10 ./visualSSS -d -secret %s -k %d -n %d -dir %s",
                     secret_path, k, n, dir);
    run_command(command);

    checked_snprintf(command, sizeof(command),
                     "./visualSSS -r -secret %s -k %d -dir %s",
                     out_path, k, dir);
    run_command(command);

    BmpImage recovered;
    assert(bmp_read(out_path, &recovered) == BMP_OK);
    assert(recovered.width == 8);
    assert(recovered.height == 1);
    assert(memcmp(recovered.pixels, secret_pixels, sizeof(secret_pixels)) == 0);
    bmp_free(&recovered);

    printf("[OK] visualSSS distribute/recover k=%d BMP E2E\n", k);
}

int main(void) {
    for (int k = 2; k <= 10; k++) {
        int n = k < 10 ? k + 1 : k;
        int shadow_len = (8 + k - 1) / k;
        int required_pixels = shadow_len * 8;
        run_e2e_case(k, n, 8, (required_pixels + 7) / 8);
    }

    return 0;
}
