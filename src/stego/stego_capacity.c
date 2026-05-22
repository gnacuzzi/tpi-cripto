#include "stego.h"

static int32_t abs_height(int32_t height) {
    return height < 0 ? -height : height;
}

static int carriers_same_dimensions(const BmpImage *carriers, size_t n) {
    if (n == 0) {
        return 0;
    }

    int32_t width = carriers[0].width;
    int32_t height = carriers[0].height;
    for (size_t i = 1; i < n; i++) {
        if (carriers[i].width != width || carriers[i].height != height) {
            return 0;
        }
    }
    return 1;
}

size_t stego_shadow_byte_count(size_t secret_pixel_count, int k) {
    if (k <= 0 || secret_pixel_count == 0) {
        return 0;
    }
    return (secret_pixel_count + (size_t)k - 1U) / (size_t)k;
}

size_t stego_required_carrier_pixels(size_t secret_pixel_count, int k) {
    return stego_shadow_byte_count(secret_pixel_count, k) * 8U;
}

StegoError stego_validate_carriers_distribute(const BmpImage *carriers, size_t n,
                                              int32_t secret_width, int32_t secret_height,
                                              int k) {
    if (carriers == NULL || n == 0 || secret_width <= 0 || secret_height <= 0 || k < 2) {
        return STEGO_ERR_ARG;
    }

    if (!carriers_same_dimensions(carriers, n)) {
        return STEGO_ERR_DIMENSIONS;
    }

    size_t secret_pixels = (size_t)secret_width * (size_t)abs_height(secret_height);
    size_t required_pixels = stego_required_carrier_pixels(secret_pixels, k);
    size_t carrier_pixels = bmp_pixel_count(&carriers[0]);

    if (k == 8) {
        if (carriers[0].width != secret_width
            || abs_height(carriers[0].height) != abs_height(secret_height)) {
            return STEGO_ERR_DIMENSIONS;
        }
    }

    if (carrier_pixels < required_pixels) {
        return STEGO_ERR_CARRIER_SIZE;
    }

    return STEGO_OK;
}

StegoError stego_validate_carriers_recover(const BmpImage *carriers, size_t count, int k) {
    if (carriers == NULL || count == 0 || k < 2) {
        return STEGO_ERR_ARG;
    }

    if (count < (size_t)k) {
        return STEGO_ERR_COUNT;
    }

    if (!carriers_same_dimensions(carriers, count)) {
        return STEGO_ERR_DIMENSIONS;
    }

    return STEGO_OK;
}
