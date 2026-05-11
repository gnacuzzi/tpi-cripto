# PLAN.md

Plan de trabajo recomendado para el TP de Secreto Compartido en Imagenes con Esteganografia.

## Diagnostico principal

El riesgo grande del TP no es escribir mucho codigo, sino integrar tarde partes que eran verificables por separado. Si arrancas directo por "distribuir y recuperar BMPs", vas a mezclar errores de formato BMP, modulo 257, LSB, metadata y reconstruccion. La estrategia correcta es construir una tuberia por capas, con tests chicos antes de tocar imagenes reales.

## Decision tomada: C

El lenguaje elegido es C. Eso tiene sentido para este TP porque el trabajo es binario, se manipulan bytes, offsets, bits y archivos BMP sin compresion. La contra es que los errores de memoria, tamanos y endianness no perdonan, asi que hay que ser estrictos desde el primer archivo.

Reglas practicas para C:

- Usar `uint8_t`, `uint16_t`, `uint32_t`, `int64_t`, `size_t`.
- Leer BMP como bytes y parsear little endian manualmente.
- Compilar con `-std=c11 -Wall -Wextra -Werror -pedantic`.
- No usar casts para tapar warnings.
- Cada modulo tiene `.h` chico, `.c` con implementacion y test propio.

## Estructura recomendada del proyecto

Mantendria una estructura equivalente a esta, adaptada al lenguaje elegido.

```text
visualSSS/
  README.md
  AGENTS.md
  PLAN.md
  docs/
    decisiones.md
    analisis-paper.md
  samples/
    secret/
    carriers/
    outputs/
  tests/
    fixtures/
```

```text
src/
  main.c
  cli.c / cli.h
  bmp.c / bmp.h
  metadata.c / metadata.h
  random_table.c / random_table.h
  gf257.c / gf257.h
  polynomial.c / polynomial.h
  sharing.c / sharing.h
  reconstruction.c / reconstruction.h
  stego_lsb.c / stego_lsb.h
  validation.c / validation.h
  errors.h
tests/
  test_gf257.c
  test_bmp.c
  test_random_table.c
  test_polynomial.c
  test_reconstruction.c
  test_stego_lsb.c
Makefile
```

La division importante no es el nombre exacto de archivos sino que BMP, algebra modulo 257 y esteganografia no queden mezclados.

## Archivos por los que arrancaria

Orden concreto:

1. `Makefile`
2. `src/gf257.h`
3. `src/gf257.c`
4. `tests/test_gf257.c`
5. `src/bmp.h`
6. `src/bmp.c`
7. `tests/test_bmp.c`
8. `src/random_table.h`
9. `src/random_table.c`
10. `tests/test_random_table.c`

Razon: `gf257` y `bmp` son las dos bases independientes del TP. Si esas estan bien, el resto se apoya sobre suelo firme. No empezaria por `main.c` ni por `cli.c`, porque te da sensacion de avance pero no valida lo dificil.

## Fase 1: cerrar decisiones de diseno

Antes de implementar, definan y escriban en `docs/decisiones.md`:

1. Como se elige la semilla en distribucion: fija por parametro, aleatoria, o default reproducible.
2. Como se seleccionan portadoras del directorio si hay mas BMPs que `n`: orden alfabetico recomendado para reproducibilidad.
3. Que bytes de imagen se consideran payload: solo pixeles reales excluyendo padding, o matriz fisica completa. Recomiendo pixeles reales.
4. Como se resuelve `k != 8`: recomiendo LSB de 1 bit universal y exigir capacidad `8 * ceil(M/k)` bytes portadores por sombra.
5. Como se guarda el tamano original para `k != 8`: recomiendo metadata propia en payload esteganografico, porque solo con dimensiones de portadoras no siempre alcanza.
6. Que hacer si `M % k != 0`: para el primer alcance, recomiendo validar que las imagenes de prueba cumplan divisibilidad; para un programa mas general, usar padding y guardar longitud original.

No patearia la decision de `k != 8`: impacta validacion, recuperacion, README y tests.

## Fase 2: BMP aislado

Objetivo: poder abrir, validar y copiar BMPs sin alterar datos.

Tareas:

1. Leer archivo binario completo.
2. Validar firma `BM`.
3. Leer ancho, alto, bits por pixel, compresion y offset de pixeles.
4. Rechazar todo lo que no sea BMP 8 bpp sin compresion.
5. Calcular cantidad de pixeles y stride si deciden excluir padding.
6. Escribir una copia identica o controladamente modificada.
7. Calcular `M`, la cantidad de bytes/pixeles que realmente entran al algoritmo, y validar divisibilidad por `k` si todavia no implementaron padding.

Prueba minima: abrir una BMP, escribir `copy.bmp`, comparar byte a byte.

Archivos involucrados en C:

- `src/bmp.h`
- `src/bmp.c`
- `tests/test_bmp.c`

## Fase 3: metadata BMP

Objetivo: escribir y leer valores en bytes 6-9 sin romper la imagen.

Tareas:

1. Guardar seed de 2 bytes en posiciones 6 y 7, little endian.
2. Guardar numero de sombra en posiciones 8 y 9, little endian.
3. Leerlos en recuperacion.

Prueba minima: seed 641 debe quedar byte 6 = `0x81`, byte 7 = `0x02`; sombra 3 debe quedar byte 8 = `0x03`, byte 9 = `0x00`.

Archivos involucrados en C:

- `src/metadata.h`
- `src/metadata.c`
- `tests/test_metadata.c`

## Fase 4: algebra modulo 257

Objetivo: que el nucleo matematico sea confiable antes de usar imagenes.

Tareas:

1. Implementar normalizacion modulo 257.
2. Implementar suma, resta, multiplicacion, potencia.
3. Implementar inverso multiplicativo para 1..256.
4. Implementar division como multiplicacion por inverso.
5. Implementar evaluacion de polinomios.
6. Implementar recuperacion de coeficientes con Lagrange reducido o Gauss.

Recomendacion: usar Lagrange/Gauss, no determinantes, tal como pide la consigna. Para testear, crear polinomios conocidos y recuperar coeficientes desde distintos subconjuntos de `k` puntos.

Archivos involucrados en C:

- `src/gf257.h`
- `src/gf257.c`
- `src/polynomial.h`
- `src/polynomial.c`
- `src/reconstruction.h`
- `src/reconstruction.c`
- `tests/test_gf257.c`
- `tests/test_polynomial.c`
- `tests/test_reconstruction.c`

## Fase 5: generacion de sombras sin BMP

Objetivo: demostrar que arrays de bytes se reparten y recuperan.

Tareas:

1. Generar R con seed.
2. Hacer XOR secreto -> Q.
3. Agrupar Q de a `k`.
4. Evaluar polinomios para sombras 1..n.
5. Ajustar si aparece 256.
6. Reconstruir Q desde cualquier subconjunto de `k` sombras.
7. Aplicar XOR inverso.

Prueba minima: arrays cortos, por ejemplo 16 o 32 bytes, con `k = 2`, `k = 3`, `k = 8`, `k = 10`.

Punto a observar: si aparece ajuste por 256, puede haber diferencia contra el original. Eso no necesariamente es bug; hay que registrarlo para el analisis.

## Fase 6: LSB aislado

Objetivo: insertar y extraer bytes sin depender de Shamir.

Tareas:

1. Insertar bits MSB a LSB en el LSB de bytes portadores.
2. Extraer en el mismo orden.
3. No escribir fuera de capacidad.
4. No tocar header ni paleta.

Prueba minima: ocultar `0xD1` y recuperar exactamente `0xD1`. La consigna da ese ejemplo de orden de bits.

## Fase 7: flujo `k = 8`

Objetivo: completar primero el caso mas especificado por la consigna.

Distribucion:

1. Validar secret BMP 8 bpp.
2. Buscar `n` portadoras BMP 8 bpp con mismo ancho y alto que la secreta.
3. Generar sombras con `k = 8`.
4. Ocultar cada sombra en una portadora con LSB.
5. Escribir seed y numero de sombra en bytes 6-9.
6. Guardar salidas con nombres deterministas.

Recuperacion:

1. Leer `k` portadoras validas.
2. Leer seed y numero de sombra.
3. Extraer sombras por LSB.
4. Reconstruir pixeles.
5. Copiar header de una portadora y escribir imagen secreta.

Este caso deberia ser el primer end-to-end real.

## Fase 8: flujo `k != 8`

Objetivo: implementar la decision propia y dejarla justificada.

Recomendacion concreta:

- Mantener LSB 1 bit por byte.
- Cada sombra tiene `ceil(M/k)` bytes.
- Cada portadora debe tener al menos `8 * shadow_size` bytes/pixeles disponibles.
- Guardar metadata de tamano secreto o dimensiones secretas para poder reconstruir sin adivinar.

Justificacion:

- Mantiene la imperceptibilidad mejor que usar varios bits LSB.
- Unifica extraccion e insercion.
- Hace clara la validacion de capacidad.
- La contra es que para `k < 8` exige portadoras mas grandes; eso es facil de explicar.

## Fase 9: CLI e integracion

Objetivo: que el programa se comporte como pide la consigna.

Validaciones CLI:

- Respetar orden de parametros.
- `-d` y `-r` son excluyentes.
- `-secret` obligatorio.
- `-k` obligatorio y `2 <= k <= 10`.
- En distribucion, `n` debe cumplir `k <= n <= 256` porque las sombras se evaluan en `x = 1..n` modulo 257.
- `-n` solo valido con `-d`.
- `-dir` opcional.
- En recuperacion, aunque la consigna muestra `-n` en un ejemplo, el texto dice que `-n` solo puede usarse con `-d`. Esto es una inconsistencia a mencionar; recomiendo seguir el texto normativo y no requerir `-n` en recuperacion.

## Fase 10: documentacion de entrega

README minimo:

- Como compilar.
- Como ejecutar distribucion.
- Como ejecutar recuperacion.
- Requisitos de BMP.
- Politica para seleccionar portadoras.
- Decision para `k != 8`.
- Limitaciones conocidas.

Informe/analisis:

- Paper Wu-Lo: estructura, algoritmo, notacion.
- Diferencias entre Thien-Lin y Wu-Lo.
- Uso de modulo 257 y problema del 256.
- Por que la recuperacion puede no ser exacta.
- Seguridad de seed y numero de sombra en bytes reservados.
- Extension a 24 bpp.
- Dificultades encontradas.
- Mejoras futuras.

## Orden recomendado de arranque

1. Crear `Makefile` minimo.
2. Implementar y testear `gf257`.
3. Conseguir o generar BMPs 8 bpp chicas y controladas.
4. Implementar lector BMP y tests de copia.
5. Implementar `random_table`.
6. Implementar polinomios y reconstruccion con tests.
7. Implementar sharing sobre arrays.
8. Implementar LSB sobre arrays/BMP.
9. Integrar `k = 8`.
10. Recien despues cerrar e implementar `k != 8`.

## Supuestos que estoy haciendo

- El lenguaje elegido es C.
- Asumo BMP 8 bpp sin compresion porque es lo que exige la consigna y evita sumar alcance no pedido.
- Asumo que conviene excluir padding de filas como secreto/portador porque son bytes de alineacion, no pixeles; si las imagenes de prueba de la catedra esperan recorrer todo desde offset hasta EOF, habria que ajustar.
- Asumo que al principio se puede exigir `M % k == 0`; si despues quieren soportar cualquier BMP 8 bpp, hay que agregar padding mas metadata de longitud real.
- Asumo que podemos agregar metadata propia para `k != 8` siempre que se documente, porque la consigna explicitamente deja ese caso a criterio del grupo.
