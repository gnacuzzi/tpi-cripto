# visualSSS — Secreto compartido en imágenes (Wu–Lo + esteganografía)

Implementación en C del esquema de Luang-Shyr Wu y Tsung-Ming Lo para el TP de Criptografía y Seguridad

## Requisitos (Pampero / Linux)

- `gcc` o `cc` con soporte C11
- `make`
- POSIX (`dirent`, `stat`) — sin librerías externas de imagen

## Build

```bash
make visualSSS   # produce ./visualSSS
make test        # tests unitarios (GF(257), PRNG, BMP)
make test-cli    # validación CLI
make clean
```

En **Pampero**, desde el directorio del proyecto:

```bash
make clean && make visualSSS && make test && make test-cli
```

## Uso de la CLI

Sintaxis (enunciado §4.1):

```text
visualSSS -d|-r -secret <file.bmp> -k <2..10> [-n <int>] [-dir <path>]
```

### Distribución (`-d`)

Oculta la imagen secreta en `n` portadoras BMP del directorio indicado.

```bash
./visualSSS -d -secret clave.bmp -k 2 -n 4 -dir varias
./visualSSS -d -secret clave.bmp -k 3              # n = cantidad de .bmp en cwd
```

### Recuperación (`-r`)

Reconstruye la imagen secreta a partir de al menos `k` portadoras.

```bash
./visualSSS -r -secret secreta.bmp -k 2 -dir varias
./visualSSS -r -secret secreta.bmp -k 3
```

### Validaciones

- `k` debe estar en `[2, 10]`.
- En `-d`: `n ≥ 2`, `k ≤ n`; el archivo `-secret` debe existir.
- `-n` solo aplica a `-d` (si se omite, `n` = cantidad de `.bmp` en `-dir` o cwd).
- Errores se reportan en **stderr** con mensaje de uso; código de salida ≠ 0.

## Módulo BMP

API en `include/bmp.h`. Soporta BMP **8 bits/píxel**, sin compresión (BI_RGB).

### Formato aceptado

- Magic `BM`, `biBitCount = 8`, `biCompression = 0`
- Offset a matriz de píxeles leído de bytes **10–13** (no se asume 54 ni 1078)
- Paleta incluida en el header cuando el offset es mayor (ej. 1078 = 54 + 256×4)

### Metadatos del esquema (enunciado §4.2.5)

Todos los campos multibyte se codifican en **little-endian**:

| Bytes | Contenido | Ejemplo |
|-------|-----------|---------|
| 6–7 | Semilla PRNG (`uint16_t`) | 641 → `81 02` |
| 8–9 | Índice de sombra `1…n` | 3 → `03 00` |
| 46–49 | Ancho del secreto (`biClrUsed`) | — |
| 50–53 | Alto del secreto (`biClrImportant`) | — |

Funciones: `bmp_get_seed` / `bmp_set_seed`, `bmp_get_shadow_index` / `bmp_set_shadow_index`, `bmp_get_secret_width` / `bmp_get_secret_height` / `bmp_set_secret_dims`.

### Convención de píxeles

El buffer `pixels` usa orden **bottom-up** (estándar BMP con `biHeight` positivo): `pixels[0..width-1]` es la fila inferior.

### I/O

```c
BmpImage img;
if (bmp_read("entrada.bmp", &img) != BMP_OK) { /* error */ }
bmp_set_seed(&img, 641);
bmp_write("salida.bmp", &img);
bmp_free(&img);
```

Para tests sin disco: `bmp_read_buffer` / `bmp_write_buffer`.

## Estructura del proyecto

```text
include/
  cli.h, bmp.h, gf257.h, permutation_table.h
src/
  main.c
  cli/              parser y validación
  bmp/              bmp_io.c, bmp_metadata.c
  gf257.c
  permutation_table.c
tests/
  test_cli.sh
  test_bmp.c
  test_gf257.c
  test_permutation_table.c
Makefile
```

## Referencias

- Enunciado: `docs/Trabajo Practico de Implementacion2026_1.md`
- Paper Wu–Lo: `docs/AMM.284-287.3025.md`
- Tabla de permutación PRNG: `docs/Tabla de Permutacion Implementacion.md`
