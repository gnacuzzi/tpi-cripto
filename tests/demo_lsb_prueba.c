#include "bmp.h"
#include "stego.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *CARRIER_DIR = "archivosdeprueba";
static const char *DEMO_CARRIER = "archivosdeprueba/Albertssd.bmp";
static const char *DEMO_OUTPUT = "build/demo_albert_embed.bmp";

static const char *ALL_CARRIERS[] = {
    "archivosdeprueba/Albertssd.bmp",
    "archivosdeprueba/Alfredssd.bmp",
    "archivosdeprueba/Audreyssd.bmp",
    "archivosdeprueba/Evassd.bmp",
    "archivosdeprueba/Facundossd.bmp",
    "archivosdeprueba/Gustavossd.bmp",
    "archivosdeprueba/Jamesssd.bmp",
    "archivosdeprueba/Marilynssd.bmp",
};

static void die(const char *msg) {
    fprintf(stderr, "demo_lsb_prueba: %s\n", msg);
    exit(1);
}

static void die_bmp(const char *path, BmpError err) {
    fprintf(stderr, "demo_lsb_prueba: %s: %s\n", path, bmp_error_string(err));
    exit(1);
}

static void die_stego(StegoError err) {
    fprintf(stderr, "demo_lsb_prueba: %s\n", stego_error_string(err));
    exit(1);
}

static int parse_k(int argc, char **argv) {
    if (argc <= 1) {
        return 8;
    }
    char *end = NULL;
    long k = strtol(argv[1], &end, 10);
    if (end == NULL || *end != '\0' || k < 2 || k > 10) {
        die("uso: demo_lsb_prueba [k]   (k entre 2 y 10, default 8)");
    }
    return (int)k;
}

static void fill_shadow(uint8_t *shadow, size_t shadow_len, uint16_t shadow_index) {
    for (size_t i = 0; i < shadow_len; i++) {
        shadow[i] = (uint8_t)((0xD1U + (uint8_t)i + (uint8_t)shadow_index) & 0xFFU);
    }
}

static void demo_d1_example(void) {
    static const uint8_t before[] = {
        0xED, 0xA4, 0x45, 0x36, 0x3A, 0x3A, 0x3A, 0x39
    };
    static const uint8_t expected[] = {
        0xED, 0xA5, 0x44, 0x37, 0x3A, 0x3A, 0x3A, 0x39
    };

    uint8_t pixels[8];
    memcpy(pixels, before, sizeof(before));

    if (lsb_embed_pixels(pixels, 8, (const uint8_t[]){0xD1}, 1) != STEGO_OK) {
        die_stego(STEGO_ERR_CAPACITY);
    }
    if (memcmp(pixels, expected, sizeof(expected)) != 0) {
        die("el ejemplo 0xD1 del enunciado no coincide con los píxeles esperados");
    }

    uint8_t extracted = 0;
    if (lsb_extract_pixels(pixels, 8, &extracted, 1) != STEGO_OK) {
        die_stego(STEGO_ERR_CAPACITY);
    }
    if (extracted != 0xD1) {
        die("extraccion del ejemplo 0xD1 fallo");
    }

    printf("[OK] ejemplo enunciado 0xD1 (MSB->LSB en 8 pixeles)\n");
}

static void demo_single_carrier(int k, int32_t secret_w, int32_t secret_h) {
    size_t secret_pixels = (size_t)secret_w * (size_t)secret_h;
    size_t shadow_len = stego_shadow_byte_count(secret_pixels, k);
    size_t required = stego_required_carrier_pixels(secret_pixels, k);

    printf("secreto simulado: %dx%d (%zu pixeles), k=%d\n",
           secret_w, secret_h, secret_pixels, k);
    printf("  bytes de sombra: %zu\n", shadow_len);
    printf("  pixeles minimos de portadora: %zu\n", required);

    BmpImage carrier;
    BmpError bmp_err = bmp_read(DEMO_CARRIER, &carrier);
    if (bmp_err != BMP_OK) {
        die_bmp(DEMO_CARRIER, bmp_err);
    }

    printf("portadora: %s (%dx%d, %zu pixeles, indice=%u, seed=%u)\n",
           DEMO_CARRIER,
           carrier.width,
           carrier.height,
           bmp_pixel_count(&carrier),
           bmp_get_shadow_index(&carrier),
           bmp_get_seed(&carrier));

    if (stego_validate_carriers_distribute(&carrier, 1, secret_w, secret_h, k) != STEGO_OK) {
        bmp_free(&carrier);
        fprintf(stderr,
                "demo_lsb_prueba: la portadora no cumple capacidad para k=%d "
                "(proba con k=8 y secreto 300x300)\n",
                k);
        exit(1);
    }

    uint8_t *shadow = malloc(shadow_len);
    uint8_t *extracted = malloc(shadow_len);
    if (shadow == NULL || extracted == NULL) {
        bmp_free(&carrier);
        free(shadow);
        free(extracted);
        die("sin memoria");
    }

    fill_shadow(shadow, shadow_len, bmp_get_shadow_index(&carrier));

    if (lsb_embed(&carrier, shadow, shadow_len) != STEGO_OK) {
        bmp_free(&carrier);
        free(shadow);
        free(extracted);
        die_stego(STEGO_ERR_CAPACITY);
    }

    if (lsb_extract(&carrier, extracted, shadow_len) != STEGO_OK) {
        bmp_free(&carrier);
        free(shadow);
        free(extracted);
        die_stego(STEGO_ERR_CAPACITY);
    }

    if (memcmp(shadow, extracted, shadow_len) != 0) {
        bmp_free(&carrier);
        free(shadow);
        free(extracted);
        die("round-trip embed/extract no coincide");
    }

    BmpError write_err = bmp_write(DEMO_OUTPUT, &carrier);
    if (write_err != BMP_OK) {
        bmp_free(&carrier);
        free(shadow);
        free(extracted);
        die_bmp(DEMO_OUTPUT, write_err);
    }

    printf("[OK] embed/extract round-trip (%zu bytes de sombra)\n", shadow_len);
    printf("[OK] portadora escrita en %s\n", DEMO_OUTPUT);

    bmp_free(&carrier);
    free(shadow);
    free(extracted);
}

static void demo_validate_all_carriers(int k, int32_t secret_w, int32_t secret_h) {
    enum { N = 8 };
    BmpImage carriers[N];

    for (size_t i = 0; i < N; i++) {
        memset(&carriers[i], 0, sizeof(carriers[i]));
        BmpError err = bmp_read(ALL_CARRIERS[i], &carriers[i]);
        if (err != BMP_OK) {
            for (size_t j = 0; j < i; j++) {
                bmp_free(&carriers[j]);
            }
            die_bmp(ALL_CARRIERS[i], err);
        }
    }

    StegoError err = stego_validate_carriers_distribute(carriers, N, secret_w, secret_h, k);
    if (err != STEGO_OK) {
        for (size_t i = 0; i < N; i++) {
            bmp_free(&carriers[i]);
        }
        die_stego(err);
    }

    err = stego_validate_carriers_recover(carriers, N, k);
    if (err != STEGO_OK) {
        for (size_t i = 0; i < N; i++) {
            bmp_free(&carriers[i]);
        }
        die_stego(err);
    }

    printf("[OK] validacion de %d portadoras en %s (distribute + recover, k=%d)\n",
           N, CARRIER_DIR, k);

    for (size_t i = 0; i < N; i++) {
        printf("  %s -> indice=%u\n", ALL_CARRIERS[i], bmp_get_shadow_index(&carriers[i]));
        bmp_free(&carriers[i]);
    }
}

int main(int argc, char **argv) {
    int k = parse_k(argc, argv);
    int32_t secret_w = 300;
    int32_t secret_h = 300;

    printf("=== demo LSB/MSB con %s ===\n\n", CARRIER_DIR);

    demo_d1_example();
    demo_single_carrier(k, secret_w, secret_h);
    demo_validate_all_carriers(k, secret_w, secret_h);

    printf("\ndemo_lsb_prueba completada con exito\n");
    return 0;
}
