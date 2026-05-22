#include "stego.h"

#include <string.h>

const char *stego_error_string(StegoError err) {
    switch (err) {
    case STEGO_OK:
        return "ok";
    case STEGO_ERR_ARG:
        return "invalid argument";
    case STEGO_ERR_CAPACITY:
        return "carrier capacity exceeded";
    case STEGO_ERR_CARRIER_SIZE:
        return "carrier too small for shadow data";
    case STEGO_ERR_DIMENSIONS:
        return "carrier dimensions mismatch";
    case STEGO_ERR_COUNT:
        return "unexpected carrier count";
    }
    return "unknown stego error";
}

static StegoError embed_byte_msb_first(uint8_t *pixels, size_t pixel_count, size_t *cursor,
                                       uint8_t byte) {
    for (int bit = 7; bit >= 0; bit--) {
        if (*cursor >= pixel_count) {
            return STEGO_ERR_CAPACITY;
        }
        int bit_val = (byte >> bit) & 1;
        pixels[*cursor] = (uint8_t)((pixels[*cursor] & 0xFEU) | (uint8_t)bit_val);
        (*cursor)++;
    }
    return STEGO_OK;
}

static StegoError extract_byte_msb_first(const uint8_t *pixels, size_t pixel_count,
                                         size_t *cursor, uint8_t *byte_out) {
    uint8_t value = 0;
    for (int bit = 7; bit >= 0; bit--) {
        if (*cursor >= pixel_count) {
            return STEGO_ERR_CAPACITY;
        }
        int bit_val = pixels[*cursor] & 1;
        value = (uint8_t)(value | (uint8_t)(bit_val << bit));
        (*cursor)++;
    }
    *byte_out = value;
    return STEGO_OK;
}

StegoError lsb_embed_pixels(uint8_t *pixels, size_t pixel_count,
                            const uint8_t *shadow, size_t shadow_len) {
    if (pixels == NULL || (shadow_len > 0 && shadow == NULL)) {
        return STEGO_ERR_ARG;
    }
    if (shadow_len * 8U > pixel_count) {
        return STEGO_ERR_CAPACITY;
    }

    size_t cursor = 0;
    for (size_t i = 0; i < shadow_len; i++) {
        StegoError err = embed_byte_msb_first(pixels, pixel_count, &cursor, shadow[i]);
        if (err != STEGO_OK) {
            return err;
        }
    }
    return STEGO_OK;
}

StegoError lsb_extract_pixels(const uint8_t *pixels, size_t pixel_count,
                              uint8_t *shadow, size_t shadow_len) {
    if (pixels == NULL || (shadow_len > 0 && shadow == NULL)) {
        return STEGO_ERR_ARG;
    }
    if (shadow_len * 8U > pixel_count) {
        return STEGO_ERR_CAPACITY;
    }

    size_t cursor = 0;
    for (size_t i = 0; i < shadow_len; i++) {
        StegoError err = extract_byte_msb_first(pixels, pixel_count, &cursor, &shadow[i]);
        if (err != STEGO_OK) {
            return err;
        }
    }
    return STEGO_OK;
}

StegoError lsb_embed(BmpImage *carrier, const uint8_t *shadow, size_t shadow_len) {
    if (carrier == NULL || carrier->pixels == NULL) {
        return STEGO_ERR_ARG;
    }
    return lsb_embed_pixels(carrier->pixels, bmp_pixel_count(carrier), shadow, shadow_len);
}

StegoError lsb_extract(const BmpImage *carrier, uint8_t *shadow, size_t shadow_len) {
    if (carrier == NULL || carrier->pixels == NULL) {
        return STEGO_ERR_ARG;
    }
    return lsb_extract_pixels(carrier->pixels, bmp_pixel_count(carrier), shadow, shadow_len);
}
