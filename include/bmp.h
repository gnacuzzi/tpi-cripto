#ifndef BMP_H
#define BMP_H

#include <stddef.h>
#include <stdint.h>

/*
 * Pixel buffer convention: bottom-up row order (standard BMP with positive
 * biHeight). pixels[0 .. width-1] is the bottom scanline; pixels[(height-1)*width
 * .. height*width-1] is the top scanline.
 */
typedef struct {
    uint8_t *header;
    size_t header_size;
    int32_t width;
    int32_t height;
    uint32_t pixel_offset;
    uint8_t *pixels;
} BmpImage;

typedef enum {
    BMP_OK = 0,
    BMP_ERR_IO,
    BMP_ERR_FORMAT,
    BMP_ERR_MEMORY,
    BMP_ERR_NOT_8BPP,
    BMP_ERR_COMPRESSED
} BmpError;

const char *bmp_error_string(BmpError err);

BmpError bmp_read(const char *path, BmpImage *out);
BmpError bmp_write(const char *path, const BmpImage *img);
void bmp_free(BmpImage *img);

BmpError bmp_read_buffer(const uint8_t *data, size_t len, BmpImage *out);
BmpError bmp_write_buffer(uint8_t **out_data, size_t *out_len, const BmpImage *img);

uint16_t bmp_get_seed(const BmpImage *img);
void bmp_set_seed(BmpImage *img, uint16_t seed);
uint16_t bmp_get_shadow_index(const BmpImage *img);
void bmp_set_shadow_index(BmpImage *img, uint16_t index);

int32_t bmp_get_secret_width(const BmpImage *img);
int32_t bmp_get_secret_height(const BmpImage *img);
void bmp_set_secret_dims(BmpImage *img, int32_t width, int32_t height);

size_t bmp_row_size(int32_t width);
size_t bmp_pixel_count(const BmpImage *img);

#endif
