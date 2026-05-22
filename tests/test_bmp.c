#include "bmp.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_ok(const char *test_name) {
    printf("[OK] %s\n", test_name);
}

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

static uint8_t *build_synthetic_bmp(int32_t width, int32_t height, uint32_t pixel_offset,
                                    const uint8_t *pixel_values, size_t pixel_count,
                                    size_t *out_len) {
    int32_t abs_height = height < 0 ? -height : height;
    size_t row_size = bmp_row_size(width);
    size_t pixel_data_size = row_size * (size_t)abs_height;
    size_t total_size = pixel_offset + pixel_data_size;

    uint8_t *buffer = calloc(total_size, 1);
    assert(buffer != NULL);

    buffer[0] = 'B';
    buffer[1] = 'M';
    write_u32_le(buffer + 2, (uint32_t)total_size);
    write_u32_le(buffer + 10, pixel_offset);

    write_u32_le(buffer + 14, 40U);
    write_u32_le(buffer + 18, (uint32_t)width);
    write_u32_le(buffer + 22, (uint32_t)height);
    write_u16_le(buffer + 26, 1U);
    write_u16_le(buffer + 28, 8U);
    write_u32_le(buffer + 30, 0U);

    if (pixel_offset > 54U) {
        for (size_t i = 0; i < 256U; i++) {
            size_t palette_offset = 54U + i * 4U;
            buffer[palette_offset] = (uint8_t)i;
            buffer[palette_offset + 1] = (uint8_t)i;
            buffer[palette_offset + 2] = (uint8_t)i;
            buffer[palette_offset + 3] = 0U;
        }
    }

    uint8_t *dst = buffer + pixel_offset;
    for (int32_t row = 0; row < abs_height; row++) {
        size_t src_offset = (size_t)row * (size_t)width;
        if (src_offset + (size_t)width <= pixel_count) {
            memcpy(dst, pixel_values + src_offset, (size_t)width);
        }
        dst += row_size;
    }

    *out_len = total_size;
    return buffer;
}

static void test_row_size(void) {
    assert(bmp_row_size(1) == 4U);
    assert(bmp_row_size(4) == 4U);
    assert(bmp_row_size(5) == 8U);
    print_ok("row_size");
}

static void test_read_offset_54(void) {
    static const uint8_t pixels[] = {
        10, 20, 30, 40,
        50, 60, 70, 80,
    };
    size_t len = 0;
    uint8_t *raw = build_synthetic_bmp(2, 2, 54U, pixels, sizeof(pixels), &len);

    BmpImage img;
    assert(bmp_read_buffer(raw, len, &img) == BMP_OK);
    assert(img.pixel_offset == 54U);
    assert(img.width == 2);
    assert(img.height == 2);
    assert(img.pixels[0] == 10);
    assert(img.pixels[3] == 40);

    bmp_free(&img);
    free(raw);
    print_ok("read offset 54");
}

static void test_read_offset_1078(void) {
    static const uint8_t pixels[] = {
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
    };
    size_t len = 0;
    uint8_t *raw = build_synthetic_bmp(3, 4, 1078U, pixels, sizeof(pixels), &len);

    BmpImage img;
    assert(bmp_read_buffer(raw, len, &img) == BMP_OK);
    assert(img.pixel_offset == 1078U);
    assert(img.width == 3);
    assert(img.height == 4);
    assert(img.header_size == 1078U);
    assert(memcmp(img.pixels, pixels, sizeof(pixels)) == 0);

    bmp_free(&img);
    free(raw);
    print_ok("read offset 1078");
}

static void test_round_trip_pixels(void) {
    enum { W = 5, H = 3 };
    uint8_t pixels[(size_t)W * (size_t)H];
    for (size_t i = 0; i < sizeof(pixels); i++) {
        pixels[i] = (uint8_t)(i * 17U + 3U);
    }

    size_t len = 0;
    uint8_t *raw = build_synthetic_bmp(W, H, 54U, pixels, sizeof(pixels), &len);

    BmpImage original;
    assert(bmp_read_buffer(raw, len, &original) == BMP_OK);

    uint8_t *written = NULL;
    size_t written_len = 0;
    assert(bmp_write_buffer(&written, &written_len, &original) == BMP_OK);

    BmpImage roundtrip;
    assert(bmp_read_buffer(written, written_len, &roundtrip) == BMP_OK);
    assert(roundtrip.width == W);
    assert(roundtrip.height == H);
    assert(memcmp(roundtrip.pixels, original.pixels, sizeof(pixels)) == 0);

    bmp_free(&roundtrip);
    bmp_free(&original);
    free(written);
    free(raw);
    print_ok("round-trip pixels");
}

static void test_metadata_endianness(void) {
    size_t len = 0;
    uint8_t pixel = 0xAAU;
    uint8_t *raw = build_synthetic_bmp(1, 1, 54U, &pixel, 1U, &len);

    BmpImage img;
    assert(bmp_read_buffer(raw, len, &img) == BMP_OK);

    bmp_set_seed(&img, 641U);
    bmp_set_shadow_index(&img, 3U);

    assert(img.header[6] == 0x81U);
    assert(img.header[7] == 0x02U);
    assert(img.header[8] == 0x03U);
    assert(img.header[9] == 0x00U);
    assert(bmp_get_seed(&img) == 641U);
    assert(bmp_get_shadow_index(&img) == 3U);

    uint8_t *written = NULL;
    size_t written_len = 0;
    assert(bmp_write_buffer(&written, &written_len, &img) == BMP_OK);
    assert(written[6] == 0x81U);
    assert(written[7] == 0x02U);
    assert(written[8] == 0x03U);
    assert(written[9] == 0x00U);

    bmp_free(&img);
    free(written);
    free(raw);
    print_ok("metadata endianness");
}

static void test_secret_dims(void) {
    size_t len = 0;
    uint8_t pixel = 0U;
    uint8_t *raw = build_synthetic_bmp(2, 2, 54U, &pixel, 1U, &len);

    BmpImage img;
    assert(bmp_read_buffer(raw, len, &img) == BMP_OK);

    bmp_set_secret_dims(&img, 320, 240);
    assert(bmp_get_secret_width(&img) == 320);
    assert(bmp_get_secret_height(&img) == 240);

    bmp_free(&img);
    free(raw);
    print_ok("secret dimensions");
}

static void test_reject_invalid(void) {
    BmpImage img;

    assert(bmp_read_buffer(NULL, 0U, &img) == BMP_ERR_FORMAT);

    uint8_t bad_magic[54] = {0};
    bad_magic[0] = 'X';
    bad_magic[1] = 'Y';
    assert(bmp_read_buffer(bad_magic, sizeof(bad_magic), &img) == BMP_ERR_FORMAT);

    uint8_t not_8bpp[54] = {0};
    not_8bpp[0] = 'B';
    not_8bpp[1] = 'M';
    write_u32_le(not_8bpp + 10, 54U);
    write_u32_le(not_8bpp + 14, 40U);
    write_u32_le(not_8bpp + 18, 1U);
    write_u32_le(not_8bpp + 22, 1U);
    write_u16_le(not_8bpp + 26, 1U);
    write_u16_le(not_8bpp + 28, 24U);
    assert(bmp_read_buffer(not_8bpp, sizeof(not_8bpp), &img) == BMP_ERR_NOT_8BPP);

    uint8_t compressed[54] = {0};
    compressed[0] = 'B';
    compressed[1] = 'M';
    write_u32_le(compressed + 10, 54U);
    write_u32_le(compressed + 14, 40U);
    write_u32_le(compressed + 18, 1U);
    write_u32_le(compressed + 22, 1U);
    write_u16_le(compressed + 26, 1U);
    write_u16_le(compressed + 28, 8U);
    write_u32_le(compressed + 30, 1U);
    assert(bmp_read_buffer(compressed, sizeof(compressed), &img) == BMP_ERR_COMPRESSED);

    print_ok("reject invalid BMP");
}

static void test_file_round_trip(void) {
    enum { W = 4, H = 2 };
    uint8_t pixels[(size_t)W * (size_t)H] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    size_t len = 0;
    uint8_t *raw = build_synthetic_bmp(W, H, 1078U, pixels, sizeof(pixels), &len);

    BmpImage img;
    assert(bmp_read_buffer(raw, len, &img) == BMP_OK);
    bmp_set_seed(&img, 641U);
    bmp_set_shadow_index(&img, 2U);

    const char *path = "build/test_roundtrip.bmp";
    assert(bmp_write(path, &img) == BMP_OK);

    BmpImage loaded;
    assert(bmp_read(path, &loaded) == BMP_OK);
    assert(loaded.pixel_offset == 1078U);
    assert(memcmp(loaded.pixels, pixels, sizeof(pixels)) == 0);
    assert(bmp_get_seed(&loaded) == 641U);
    assert(bmp_get_shadow_index(&loaded) == 2U);

    bmp_free(&loaded);
    bmp_free(&img);
    free(raw);
    print_ok("file round-trip");
}

int main(void) {
    test_row_size();
    test_read_offset_54();
    test_read_offset_1078();
    test_round_trip_pixels();
    test_metadata_endianness();
    test_secret_dims();
    test_reject_invalid();
    test_file_round_trip();
    printf("bmp tests passed\n");
    return 0;
}
