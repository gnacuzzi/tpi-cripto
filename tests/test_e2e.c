#include "bmp.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

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

int main(void) {
    char dir[128];
    snprintf(dir, sizeof(dir), "build/e2e_%ld", (long)getpid());
    assert(mkdir(dir, 0700) == 0);

    static const uint8_t secret_pixels[] = {
        186, 114, 65, 105, 15, 172, 62, 94
    };
    static const uint8_t carrier_pixels[] = {
        10, 20, 30, 40, 50, 60, 70, 80
    };

    char secret_path[128];
    char out_path[128];
    snprintf(secret_path, sizeof(secret_path), "%s/secret.bmp", dir);
    snprintf(out_path, sizeof(out_path), "%s/out.bmp", dir);

    write_fixture_bmp(secret_path, 8, 1, secret_pixels);

    for (int i = 1; i <= 8; i++) {
        char carrier_path[128];
        snprintf(carrier_path, sizeof(carrier_path), "%s/carrier%d.bmp", dir, i);
        write_fixture_bmp(carrier_path, 8, 1, carrier_pixels);
    }

    char command[512];
    snprintf(command, sizeof(command),
             "VISUALSSS_SEED=10 ./visualSSS -d -secret %s -k 8 -n 8 -dir %s",
             secret_path, dir);
    run_command(command);

    snprintf(command, sizeof(command),
             "./visualSSS -r -secret %s -k 8 -dir %s",
             out_path, dir);
    run_command(command);

    BmpImage recovered;
    assert(bmp_read(out_path, &recovered) == BMP_OK);
    assert(recovered.width == 8);
    assert(recovered.height == 1);
    assert(memcmp(recovered.pixels, secret_pixels, sizeof(secret_pixels)) == 0);
    bmp_free(&recovered);

    printf("[OK] visualSSS distribute/recover k=8 BMP E2E\n");
    return 0;
}
