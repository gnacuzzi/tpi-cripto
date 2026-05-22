#include "bmp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BMP_MAGIC_0 'B'
#define BMP_MAGIC_1 'M'
#define BMP_FILE_HEADER_SIZE 14
#define BMP_DIB_HEADER_SIZE 40

static uint16_t read_u16_le(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static uint32_t read_u32_le(const uint8_t *p) {
    return (uint32_t)p[0]
        | ((uint32_t)p[1] << 8)
        | ((uint32_t)p[2] << 16)
        | ((uint32_t)p[3] << 24);
}

static int32_t read_i32_le(const uint8_t *p) {
    return (int32_t)read_u32_le(p);
}

static void write_u32_le(uint8_t *p, uint32_t value) {
    p[0] = (uint8_t)(value & 0xFFU);
    p[1] = (uint8_t)((value >> 8) & 0xFFU);
    p[2] = (uint8_t)((value >> 16) & 0xFFU);
    p[3] = (uint8_t)((value >> 24) & 0xFFU);
}

const char *bmp_error_string(BmpError err) {
    switch (err) {
    case BMP_OK:
        return "ok";
    case BMP_ERR_IO:
        return "I/O error";
    case BMP_ERR_FORMAT:
        return "invalid or corrupt BMP";
    case BMP_ERR_MEMORY:
        return "out of memory";
    case BMP_ERR_NOT_8BPP:
        return "BMP must be 8 bits per pixel";
    case BMP_ERR_COMPRESSED:
        return "BMP compression is not supported";
    }
    return "unknown BMP error";
}

size_t bmp_row_size(int32_t width) {
    if (width <= 0) {
        return 0;
    }
    return ((size_t)width + 3U) & ~3U;
}

size_t bmp_pixel_count(const BmpImage *img) {
    if (img == NULL || img->width <= 0 || img->height <= 0) {
        return 0;
    }
    return (size_t)img->width * (size_t)img->height;
}

static BmpError validate_header(const uint8_t *header, size_t header_len, size_t total_len,
                                int32_t *width_out, int32_t *height_out,
                                uint32_t *pixel_offset_out) {
    if (header == NULL || header_len < BMP_FILE_HEADER_SIZE + BMP_DIB_HEADER_SIZE) {
        return BMP_ERR_FORMAT;
    }

    if (header[0] != BMP_MAGIC_0 || header[1] != BMP_MAGIC_1) {
        return BMP_ERR_FORMAT;
    }

    uint32_t dib_size = read_u32_le(header + 14);
    if (dib_size < BMP_DIB_HEADER_SIZE) {
        return BMP_ERR_FORMAT;
    }

    if (header_len < BMP_FILE_HEADER_SIZE + dib_size) {
        return BMP_ERR_FORMAT;
    }

    uint32_t pixel_offset = read_u32_le(header + 10);
    if (pixel_offset < BMP_FILE_HEADER_SIZE + dib_size || pixel_offset > total_len) {
        return BMP_ERR_FORMAT;
    }

    int32_t width = read_i32_le(header + 18);
    int32_t height = read_i32_le(header + 22);
    if (width <= 0 || height == 0) {
        return BMP_ERR_FORMAT;
    }

    uint16_t planes = read_u16_le(header + 26);
    if (planes != 1U) {
        return BMP_ERR_FORMAT;
    }

    uint16_t bpp = read_u16_le(header + 28);
    if (bpp != 8U) {
        return BMP_ERR_NOT_8BPP;
    }

    uint32_t compression = read_u32_le(header + 30);
    if (compression != 0U) {
        return BMP_ERR_COMPRESSED;
    }

    int32_t abs_height = height < 0 ? -height : height;
    size_t row_size = bmp_row_size(width);
    size_t pixel_data_size = row_size * (size_t)abs_height;
    if (pixel_offset + pixel_data_size > total_len) {
        return BMP_ERR_FORMAT;
    }

    *width_out = width;
    *height_out = height;
    *pixel_offset_out = pixel_offset;
    return BMP_OK;
}

static BmpError read_pixels_from_source(const uint8_t *source, size_t source_len,
                                        uint32_t pixel_offset, int32_t width, int32_t height,
                                        uint8_t **pixels_out) {
    int32_t abs_height = height < 0 ? -height : height;
    size_t pixel_count = (size_t)width * (size_t)abs_height;
    size_t row_size = bmp_row_size(width);

    uint8_t *pixels = calloc(pixel_count, 1);
    if (pixels == NULL) {
        return BMP_ERR_MEMORY;
    }

    const uint8_t *row_src = source + pixel_offset;
    for (int32_t row = 0; row < abs_height; row++) {
        if (row_src + row_size > source + source_len) {
            free(pixels);
            return BMP_ERR_FORMAT;
        }
        memcpy(pixels + (size_t)row * (size_t)width, row_src, (size_t)width);
        row_src += row_size;
    }

    *pixels_out = pixels;
    return BMP_OK;
}

static BmpError parse_bmp_buffer(const uint8_t *data, size_t len, BmpImage *out) {
    if (out == NULL) {
        return BMP_ERR_FORMAT;
    }

    memset(out, 0, sizeof(*out));

    int32_t width = 0;
    int32_t height = 0;
    uint32_t pixel_offset = 0;
    BmpError err = validate_header(data, len, len, &width, &height, &pixel_offset);
    if (err != BMP_OK) {
        return err;
    }

    uint8_t *header = malloc(pixel_offset);
    if (header == NULL) {
        return BMP_ERR_MEMORY;
    }
    memcpy(header, data, pixel_offset);

    uint8_t *pixels = NULL;
    err = read_pixels_from_source(data, len, pixel_offset, width, height, &pixels);
    if (err != BMP_OK) {
        free(header);
        return err;
    }

    out->header = header;
    out->header_size = pixel_offset;
    out->width = width;
    out->height = height;
    out->pixel_offset = pixel_offset;
    out->pixels = pixels;
    return BMP_OK;
}

BmpError bmp_read_buffer(const uint8_t *data, size_t len, BmpImage *out) {
    if (data == NULL || len < BMP_FILE_HEADER_SIZE + BMP_DIB_HEADER_SIZE) {
        return BMP_ERR_FORMAT;
    }
    return parse_bmp_buffer(data, len, out);
}

BmpError bmp_read(const char *path, BmpImage *out) {
    if (path == NULL || out == NULL) {
        return BMP_ERR_FORMAT;
    }

    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return BMP_ERR_IO;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return BMP_ERR_IO;
    }

    long file_size_long = ftell(file);
    if (file_size_long < 0) {
        fclose(file);
        return BMP_ERR_IO;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return BMP_ERR_IO;
    }

    size_t file_size = (size_t)file_size_long;
    uint8_t *buffer = malloc(file_size);
    if (buffer == NULL) {
        fclose(file);
        return BMP_ERR_MEMORY;
    }

    size_t read_count = fread(buffer, 1, file_size, file);
    fclose(file);
    if (read_count != file_size) {
        free(buffer);
        return BMP_ERR_IO;
    }

    BmpError err = parse_bmp_buffer(buffer, file_size, out);
    free(buffer);
    return err;
}

static BmpError build_file_buffer(const BmpImage *img, uint8_t **out_data, size_t *out_len) {
    if (img == NULL || img->header == NULL || img->pixels == NULL
        || img->width <= 0 || img->height == 0 || img->header_size < 54U) {
        return BMP_ERR_FORMAT;
    }

    int32_t abs_height = img->height < 0 ? -img->height : img->height;
    size_t row_size = bmp_row_size(img->width);
    size_t pixel_data_size = row_size * (size_t)abs_height;
    size_t total_size = img->header_size + pixel_data_size;

    uint8_t *buffer = malloc(total_size);
    if (buffer == NULL) {
        return BMP_ERR_MEMORY;
    }

    memcpy(buffer, img->header, img->header_size);
    write_u32_le(buffer + 2, (uint32_t)total_size);

    const uint8_t *row_src = img->pixels;
    uint8_t *row_dst = buffer + img->header_size;
    for (int32_t row = 0; row < abs_height; row++) {
        memcpy(row_dst, row_src, (size_t)img->width);
        if (row_size > (size_t)img->width) {
            memset(row_dst + img->width, 0, row_size - (size_t)img->width);
        }
        row_src += (size_t)img->width;
        row_dst += row_size;
    }

    *out_data = buffer;
    *out_len = total_size;
    return BMP_OK;
}

BmpError bmp_write_buffer(uint8_t **out_data, size_t *out_len, const BmpImage *img) {
    if (out_data == NULL || out_len == NULL) {
        return BMP_ERR_FORMAT;
    }
    *out_data = NULL;
    *out_len = 0;
    return build_file_buffer(img, out_data, out_len);
}

BmpError bmp_write(const char *path, const BmpImage *img) {
    if (path == NULL) {
        return BMP_ERR_FORMAT;
    }

    uint8_t *buffer = NULL;
    size_t len = 0;
    BmpError err = build_file_buffer(img, &buffer, &len);
    if (err != BMP_OK) {
        return err;
    }

    FILE *file = fopen(path, "wb");
    if (file == NULL) {
        free(buffer);
        return BMP_ERR_IO;
    }

    size_t written = fwrite(buffer, 1, len, file);
    fclose(file);
    free(buffer);

    if (written != len) {
        return BMP_ERR_IO;
    }
    return BMP_OK;
}

void bmp_free(BmpImage *img) {
    if (img == NULL) {
        return;
    }
    free(img->header);
    free(img->pixels);
    memset(img, 0, sizeof(*img));
}
