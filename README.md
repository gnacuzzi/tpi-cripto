# visualSSS — Secreto compartido en imágenes (Wu–Lo + esteganografía)

Implementación en C del esquema de Luang-Shyr Wu y Tsung-Ming Lo para el TP de Criptografía y Seguridad.

## Estado del proyecto

| Componente | Estado |
|------------|--------|
| CLI (`-d` / `-r`, validaciones) | Implementado |
| BMP 8 bpp (lectura/escritura, metadatos) | Implementado |
| Esteganografía LSB (embed/extract, capacidad) | Implementado |
| Aritmética GF(257) | Implementado |
| PRNG / tabla de permutación | Implementado |
| Núcleo Wu–Lo (Shamir, Lagrange) | Implementado |
| Distribución y recuperación end-to-end | Implementado para k = 2..10 |

El ejecutable `visualSSS` distribuye y recupera BMP reales para todos los valores de `k` aceptados por la consigna (`2..10`). Para `k = 8`, las portadoras deben tener el mismo tamaño que el secreto. Para `k != 8`, se usa el criterio de capacidad LSB documentado abajo y se guardan las dimensiones originales del secreto en los metadatos de las sombras.

## Requisitos (Pampero / Linux)

- `gcc` o `cc` con soporte C11
- `make`
- POSIX (`dirent`, `stat`) — sin librerías externas de imagen

## Build

```bash
make visualSSS   # produce ./visualSSS
make test        # tests unitarios
make test-cli    # validación CLI
make clean
```

En **Pampero**, desde el directorio del proyecto:

```bash
make clean && make visualSSS && make test && make test-cli
```

### Tests unitarios

| Target | Cubre |
|--------|-------|
| `test_gf257` | Operaciones mod 257 (add, mul, inv, div) |
| `test_permutation_table` | Secuencia PRNG (semillas del anexo) |
| `test_bmp` | Offsets 54/1078, round-trip, metadatos, rechazo de formatos inválidos |
| `test_lsb` | Ejemplo `0xD1` del enunciado, embed/extract, capacidad y validación de portadoras |

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

## Módulo de esteganografía (LSB)

API en `include/stego.h`. Oculta bytes de sombra en el bit menos significativo de los píxeles de una portadora.

### Algoritmo

- **1 bit por píxel** de portadora (LSB replacement).
- Bits de cada byte de sombra en orden **MSB → LSB** (bit 7 primero), según la tabla del enunciado con `0xD1`.
- Recorrido lineal sobre el buffer de píxeles (`width × height`, bottom-up).

### Capacidad

```
bytes_de_sombra   = ceil(píxeles_secreto / k)
píxeles_mínimos   = bytes_de_sombra × 8
```

Con **k = 8**, las portadoras deben tener el mismo tamaño que el secreto. Con **k ≠ 8**, deben cumplir `width × height ≥ píxeles_mínimos` (mismas dimensiones entre portadoras).

### API principal

```c
size_t shadow_len = stego_shadow_byte_count(secret_pixels, k);
size_t required   = stego_required_carrier_pixels(secret_pixels, k);

lsb_embed(&carrier, shadow_bytes, shadow_len);
lsb_extract(&carrier, shadow_bytes, shadow_len);

stego_validate_carriers_distribute(carriers, n, secret_w, secret_h, k);
stego_validate_carriers_recover(carriers, count, k);
```

Variantes sobre buffers sintéticos: `lsb_embed_pixels` / `lsb_extract_pixels`.

## Base criptográfica

### GF(257) — `include/gf257.h`

Operaciones enteras en **Z/257Z**: `gf257_add`, `gf257_sub`, `gf257_mul`, `gf257_div`, `gf257_inverse`, `gf257_pow`. Sin aritmética de punto flotante.

### PRNG — `include/permutation_table.h`

Generador compatible con el anexo *Tabla de Permutación de Implementación*:

```c
PermutationTable table;
permutation_table_set_seed(&table, 641);
permutation_table_fill(&table, buffer, length);
```

Se usará para calcular `Q[i] = O[i] XOR R[i]` en la distribución.

## Estructura del proyecto

```text
include/
  cli.h, bmp.h, stego.h, gf257.h, permutation_table.h
src/
  main.c
  cli/              parse.c — parser y validación
  bmp/              bmp_io.c, bmp_metadata.c
  stego/            lsb.c, stego_capacity.c
  gf257.c
  permutation_table.c
tests/
  test_cli.sh
  test_bmp.c
  test_lsb.c
  test_gf257.c
  test_permutation_table.c
Makefile
```

## Referencias

- Enunciado: `docs/Trabajo Practico de Implementacion2026_1.md`
- Paper Wu–Lo: `docs/AMM.284-287.3025.md`
- Tabla de permutación PRNG: `docs/Tabla de Permutacion Implementacion.md`
