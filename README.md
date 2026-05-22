#TP de implementacion en C del esquema Wu–Lo para el TP de Criptografía y Seguridad

## Requisitos (Pampero / Linux)

- `gcc` o `cc` con soporte C11
- `make`
- POSIX (`dirent`, `stat`) — sin librerías externas

## Build

```bash
make visualSSS   # produce ./visualSSS
make test        # tests unitarios (GF(257), PRNG)
make test-cli    # checklist de validación CLI (Fase 1)
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
- `-n` solo aplica a `-d` (si se omite, `n` = cantidad de `.bmp` en `-dir`).
- Errores se reportan en **stderr** con mensaje de uso; código de salida ≠ 0.

## Estructura del proyecto

```text
include/          headers públicos
src/
  main.c          punto de entrada
  cli/            parser y validación
  gf257.c         aritmética mod 257 (Fase 4)
  permutation_table.c
tests/            tests unitarios y smoke CLI
```

