#include "bmp.h"

#include <stddef.h>
#include <stdint.h>

static uint16_t read_u16_le(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static void write_u16_le(uint8_t *p, uint16_t value) {
    p[0] = (uint8_t)(value & 0xFFU);
    p[1] = (uint8_t)((value >> 8) & 0xFFU);
}

static uint32_t read_u32_le(const uint8_t *p) {
    return (uint32_t)p[0]
        | ((uint32_t)p[1] << 8)
        | ((uint32_t)p[2] << 16)
        | ((uint32_t)p[3] << 24);
}

static void write_u32_le(uint8_t *p, uint32_t value) {
    p[0] = (uint8_t)(value & 0xFFU);
    p[1] = (uint8_t)((value >> 8) & 0xFFU);
    p[2] = (uint8_t)((value >> 16) & 0xFFU);
    p[3] = (uint8_t)((value >> 24) & 0xFFU);
}

uint16_t bmp_get_seed(const BmpImage *img) {
    if (img == NULL || img->header == NULL || img->header_size < 8U) {
        return 0;
    }
    return read_u16_le(img->header + 6);
}

void bmp_set_seed(BmpImage *img, uint16_t seed) {
    if (img == NULL || img->header == NULL || img->header_size < 8U) {
        return;
    }
    write_u16_le(img->header + 6, seed);
}

uint16_t bmp_get_shadow_index(const BmpImage *img) {
    if (img == NULL || img->header == NULL || img->header_size < 10U) {
        return 0;
    }
    return read_u16_le(img->header + 8);
}

void bmp_set_shadow_index(BmpImage *img, uint16_t index) {
    if (img == NULL || img->header == NULL || img->header_size < 10U) {
        return;
    }
    write_u16_le(img->header + 8, index);
}

int32_t bmp_get_secret_width(const BmpImage *img) {
    if (img == NULL || img->header == NULL || img->header_size < 50U) {
        return 0;
    }
    return (int32_t)read_u32_le(img->header + 46);
}

int32_t bmp_get_secret_height(const BmpImage *img) {
    if (img == NULL || img->header == NULL || img->header_size < 54U) {
        return 0;
    }
    return (int32_t)read_u32_le(img->header + 50);
}

void bmp_set_secret_dims(BmpImage *img, int32_t width, int32_t height) {
    if (img == NULL || img->header == NULL || img->header_size < 54U) {
        return;
    }
    write_u32_le(img->header + 46, (uint32_t)width);
    write_u32_le(img->header + 50, (uint32_t)height);
}
