#ifndef STEGO_H
#define STEGO_H

#include "bmp.h"

#include <stddef.h>
#include <stdint.h>

typedef enum {
    STEGO_OK = 0,
    STEGO_ERR_ARG,
    STEGO_ERR_CAPACITY,
    STEGO_ERR_CARRIER_SIZE,
    STEGO_ERR_DIMENSIONS,
    STEGO_ERR_COUNT
} StegoError;

const char *stego_error_string(StegoError err);

size_t stego_shadow_byte_count(size_t secret_pixel_count, int k);
size_t stego_required_carrier_pixels(size_t secret_pixel_count, int k);

StegoError lsb_embed_pixels(uint8_t *pixels, size_t pixel_count,
                            const uint8_t *shadow, size_t shadow_len);
StegoError lsb_extract_pixels(const uint8_t *pixels, size_t pixel_count,
                              uint8_t *shadow, size_t shadow_len);

StegoError lsb_embed(BmpImage *carrier, const uint8_t *shadow, size_t shadow_len);
StegoError lsb_extract(const BmpImage *carrier, uint8_t *shadow, size_t shadow_len);

StegoError stego_validate_carriers_distribute(const BmpImage *carriers, size_t n,
                                              int32_t secret_width, int32_t secret_height,
                                              int k);
StegoError stego_validate_carriers_recover(const BmpImage *carriers, size_t count, int k);

#endif
