# AGENTS.md

Contexto persistente para trabajar en el TP de Criptografia y Seguridad 72.04, primer cuatrimestre 2026.

## Forma de trabajo

- Usar siempre `skills.md` como contexto de conversacion: respuestas directas, criticas, accionables y con voseo rioplatense.
- El lenguaje elegido para el TP es C. No implementar codigo hasta que la usuaria pida avanzar con una parte concreta.
- Separar el TP en componentes verificables. La clase hizo enfasis en poder probar cada parte aisladamente antes de integrar.
- Distinguir siempre entre:
  - requisito de la consigna,
  - decision de diseno del grupo,
  - supuesto temporal de trabajo.
- Si algo depende de `k != 8`, tratarlo como decision de diseno a justificar en README e informe.

## Archivos fuente leidos

- `Trabajo Practico de Implementacion2026_1.md`: consigna principal.
- `clase.md`: transcripcion de presentacion del TP. Esta en una sola linea larga aunque `wc` marque 0 lineas.
- `AMM.284-287.3025.md`: paper pedido, "An Efficient Secret Image Sharing Scheme" de Kuang-Shyr Wu y Tsung-Ming Lo.
- `secret image sharing.md`: paper Thien-Lin 2002, base del esquema de imagenes con sombras de tamano reducido.
- `Apunte sobre Secreto Compartido.md`: marco teorico de Shamir/Blakley, umbrales, interpolacion y seguridad.
- `Tabla de Permutacion Implementacion.md`: generador pseudoaleatorio que debe usarse en C para tabla R.
- `skills.md`: preferencias de conversacion y feedback.

## Objetivo del programa

Programa CLI en C llamado conceptualmente `visualSSS` que permita:

- Distribuir (`-d`) una imagen secreta BMP de 8 bits por pixel en `n` imagenes portadoras BMP, generando sombras ocultas por esteganografia.
- Recuperar (`-r`) una imagen secreta BMP a partir de `k` imagenes BMP que contienen sombras.

Parametros segun consigna:

- Obligatorios y en orden: `-d` o `-r`, `-secret imagen`, `-k numero`.
- Opcionales: `-n numero`, `-dir directorio`.
- `-n` solo se usa en distribucion. Si falta, `n` es la cantidad de imagenes BMP disponibles en el directorio.
- `-dir` indica directorio de portadoras/sombras. Si falta, usar directorio actual.

Validaciones principales:

- `k` entre 2 y 10 inclusive.
- En distribucion, `k <= n`.
- `n >= 2`.
- Como las evaluaciones usan `x = 1..n` en el cuerpo finito modulo 257, debe mantenerse `n <= 256` salvo que se cambie formalmente el esquema. Recomendacion: validar `2 <= n <= 256`.
- Imagen secreta y portadoras deben ser BMP de 8 bits por pixel.
- No tocar encabezados salvo los bytes reservados indicados para metadata y el encabezado de salida al reconstruir.

## BMP: puntos que no se pueden negociar

- BMP es binario.
- No asumir que los pixeles empiezan en byte 54. Leer el campo `bfOffBits` del encabezado para obtener el offset real de la matriz de pixeles.
- La consigna habla de encabezado de 54 bytes, pero advierte que la matriz de pixeles puede empezar mas adelante, por ejemplo por paleta en imagenes de 8 bpp.
- Para 8 bpp, cada pixel ocupa 1 byte, pero BMP suele tener filas alineadas a multiplos de 4 bytes. Hay que decidir explicitamente si el algoritmo trabaja sobre todos los bytes de la matriz desde `bfOffBits` hasta EOF o solo sobre pixeles reales excluyendo padding. Recomendacion: parsear ancho, alto y stride, y tratar el padding como bytes no-secretos/no-portadores salvo que la consigna/pruebas indiquen otra cosa.
- La imagen se almacena de abajo hacia arriba y de izquierda a derecha si el alto BMP es positivo. Para este TP puede bastar con preservar el orden fisico de bytes de la matriz, pero documentarlo.
- Revisar que no haya metadata extra al final en imagenes de prueba, como advierte la consigna.

## Algoritmo criptografico

El esquema implementa la idea de Thien-Lin modificada por Wu-Lo:

1. Generar tabla pseudoaleatoria `R` de longitud igual a la cantidad de pixeles/bytes secretos procesados, con valores `[0,255]`.
2. Randomizar la imagen secreta: `Q[i] = O[i] XOR R[i]`.
3. Tomar la imagen randomizada de a `k` bytes: `a0, a1, ..., a(k-1)`.
4. Formar un polinomio de grado `k-1`:
   `f(x) = a0 + a1*x + ... + a(k-1)*x^(k-1) mod 257`.
5. Evaluar el polinomio en `x = 1..n`. El valor para `x = i` es el byte de la sombra numero `i`.
6. Si alguna evaluacion da `256`, se debe ajustar el primer coeficiente no cero decrementandolo en 1 y recalcular hasta que todas las salidas esten en `[0,255]`.
7. En recuperacion, con cualquier `k` sombras, se reconstruyen los coeficientes del polinomio en modulo 257 usando Lagrange reducido o Gauss. La consigna recomienda no usar determinantes.
8. Aplicar XOR otra vez con la misma `R` para obtener la imagen recuperada.

Notas importantes:

- Todo el algebra de generacion y recuperacion se hace modulo 257, primo.
- Si la cantidad de bytes secretos `M` no es multiplo de `k`, hay que decidir como completar el ultimo bloque. Recomendacion para no agregar complejidad al inicio: validar y usar imagenes donde `M % k == 0` para el primer end-to-end; si se necesita soportar cualquier tamano, agregar padding y guardar longitud original para truncar al recuperar.
- Las sombras contienen bytes, por eso no se puede guardar 256 directamente.
- Si hubo ajuste por salida 256, la recuperacion puede no ser exactamente igual al original. Esa es una pregunta explicita del informe.
- Los valores `x` de las sombras no son la posicion del archivo sino el numero de sombra guardado en metadata.

## C: criterios de implementacion

- Usar archivos `.c` y `.h` separados por responsabilidad.
- Evitar `struct` mapeadas directamente sobre el archivo BMP si eso introduce padding/alineacion no controlada. Preferir leer bytes y convertir campos little endian explicitamente.
- Usar tipos de ancho fijo: `uint8_t`, `uint16_t`, `uint32_t`, `int32_t`, `int64_t`, `size_t`.
- Toda apertura de BMP debe ser binaria: `rb` para leer, `wb` para escribir.
- Cada funcion que pueda fallar debe devolver codigo de error o `bool` y dejar el detalle en un mensaje controlado. No mezclar `printf` de debug con la logica interna.
- No usar variables globales salvo que se copie literalmente el RNG del apunte; aun ahi conviene encapsular estado si no complica.
- Compilar con flags estrictos desde el principio: `-std=c11 -Wall -Wextra -Werror -pedantic`.
- Cuidar overflow en calculos de tamanos: ancho * alto, stride, capacidad LSB y `8 * shadow_size`.

## Tabla R y semilla

- La tabla R se genera con una semilla de 2 bytes.
- La semilla se guarda en bytes 6 y 7 del BMP portador/sombra, en little endian segun ejemplo de la consigna:
  - seed 641 decimal = `0x0281`,
  - byte 6 = `0x81`,
  - byte 7 = `0x02`.
- En C, usar el generador especificado en `Tabla de Permutacion Implementacion.md`:
  - estado global de 48 bits,
  - `setSeed(s) = (s ^ 0x5DEECE66D) & ((1 << 48) - 1)`,
  - `nextChar()` actualiza con multiplicador `0x5DEECE66D`, suma `0xB` y devuelve `seed >> 40`.
- La consigna llama "tabla de permutacion" a `R`, pero el paper Wu-Lo realmente aplica XOR con una tabla pseudoaleatoria, no una permutacion de posiciones como Thien-Lin. Conviene explicarlo en el informe porque la notacion puede confundir.

## Esteganografia

Caso `k = 8`:

- Las portadoras deben tener mismo ancho y alto que la imagen secreta.
- Si no hay `n` imagenes que cumplan, error y no modificar nada.
- Ocultar cada byte de sombra por LSB replacement, 1 bit por byte/pixel portador.
- Insertar bits de mayor a menor, desde el primer pixel real (`bfOffBits`).
- Con `k = 8`, cada sombra mide `M/8` bytes si la imagen secreta tiene `M` bytes procesados. Ocultarla a 1 bit por pixel requiere `M` bytes portadores, por eso alcanza una portadora del mismo tamano que la secreta.
- Esta igualdad solo cierra perfecto si `M` es multiplo de 8. Si no lo es, hay que exigir imagenes con `M % 8 == 0` o definir padding y capacidad extra. Para una primera implementacion robusta ante la catedra, conviene elegir/validar imagenes con dimensiones que hagan `M % 8 == 0`.

Metadata en bytes reservados del BMP:

- Bytes 6 y 7: semilla de R, little endian.
- Bytes 8 y 9: numero de sombra, little endian. Ejemplo: sombra 3 => byte 8 = `0x03`, byte 9 = `0x00`.

Caso `k != 8`:

- La consigna deja a criterio del grupo el tamano de portadoras y el metodo de ocultamiento.
- Hay que documentar requerimientos especiales en README.
- Decision recomendada para simplicidad y consistencia: mantener LSB replacement de 1 bit por byte portador para todos los `k`. Entonces cada sombra de `ceil(M/k)` bytes necesita portadora con al menos `8 * ceil(M/k)` bytes/pixeles disponibles. Esto implica:
  - si `k < 8`, la portadora debe ser mas grande que la secreta;
  - si `k = 8`, mismo tamano;
  - si `k > 8`, podria ser mas chica, pero aceptar portadoras iguales o mas grandes simplifica.
- Alternativa descartable pero discutible: usar mas de 1 LSB por byte para permitir portadoras mas chicas cuando `k < 8`. Es mas invasivo visualmente y requiere justificar impacto esteganografico.

## Recuperacion

Caso `k = 8`:

- Se necesitan al menos 8 imagenes validas del mismo ancho y alto entre si.
- Leer semilla y numero de sombra de cada portadora.
- Extraer LSBs en el mismo orden usado para embebido.
- Reagrupar cada 8 bits en un byte de sombra.
- Reconstruir imagen secreta del mismo tamano que las portadoras. Para encabezado, se puede copiar el de una portadora.

Caso `k != 8`:

- La validacion y regeneracion del tamano dependen de la decision de capacidad elegida.
- Si se usa la recomendacion de 1 LSB universal, hay que tener forma de saber cuantos bytes secretos reconstruir. Opciones:
  - exigir portadoras con dimensiones que permitan deducir `M` desde `capacity * k / 8`;
  - guardar metadata adicional de ancho/alto/tamano secreto;
  - requerir portadoras generadas por el mismo programa y documentar el formato.
- Recomendacion: guardar metadata minima adicional en un pequeno header propio dentro del payload esteganografico o en bytes reservados extendidos si se justifica. No depender de adivinar dimensiones.
- Si se adopta padding para soportar `M % k != 0`, la metadata debe incluir la longitud real original.

## Estructura C recomendada

Separar responsabilidades en pares `.c/.h`:

- `main.c`: punto de entrada; solo orquesta parser, validacion y servicios.
- `cli.c` / `cli.h`: parseo estricto de argumentos y validacion de combinaciones.
- `bmp.c` / `bmp.h`: lectura/escritura BMP, header, offset de pixeles, bpp, ancho/alto, stride, copia de header.
- `metadata.c` / `metadata.h`: lectura/escritura de seed, numero de sombra y, si se decide, metadata extra.
- `random_table.c` / `random_table.h`: generacion de R del apunte.
- `gf257.c` / `gf257.h`: suma, resta, multiplicacion, potencia, inverso y division modulo 257.
- `polynomial.c` / `polynomial.h`: evaluacion de polinomios y ajuste ante salida 256.
- `sharing.c` / `sharing.h`: distribucion de bytes secretos randomizados a bytes de sombra.
- `reconstruction.c` / `reconstruction.h`: interpolacion/Gauss modulo 257 para recuperar coeficientes.
- `stego_lsb.c` / `stego_lsb.h`: insercion y extraccion LSB.
- `validation.c` / `validation.h`: seleccion de portadoras validas por extension, bpp, dimensiones y capacidad.
- `errors.h`: enum comun de errores si ayuda a no duplicar manejo de fallos.

Archivos de arranque recomendados:

- `Makefile`: compilar binario y tests con flags estrictos.
- `src/gf257.c` / `src/gf257.h`: primer modulo matematico, facil de testear sin BMP.
- `src/bmp.c` / `src/bmp.h`: primer modulo binario, necesario para todo el TP.
- `tests/test_gf257.c` y `tests/test_bmp.c`: pruebas chicas sin framework complejo.

## Pruebas por etapas

Orden de pruebas recomendado:

1. Parser CLI con casos validos/invalidos.
2. BMP: abrir BMP 8 bpp, leer `bfOffBits`, ancho, alto, bpp, tamanos y reescribir sin cambios.
3. Metadata: escribir/leer seed y numero de sombra en bytes 6-9.
4. RNG: comparar primeros valores para seed 10 contra la secuencia esperada del generador del apunte.
5. GF(257): verificar inversos y divisiones.
6. Polinomios: evaluar casos manuales y validar que nunca se emita 256.
7. Lagrange/Gauss: recuperar coeficientes conocidos desde `k` puntos.
8. LSB: esconder y extraer un byte conocido, por ejemplo `0xD1`, respetando orden MSB a LSB.
9. Sharing sin BMP: array de bytes secreto -> sombras -> recuperacion.
10. Flujo `k = 8` con BMP sinteticas chicas.
11. Flujo `k != 8` luego de cerrar la decision de capacidad.
12. Comparacion byte a byte original vs recuperada y analisis de diferencias por ajuste de 256.

## Puntos para README/informe

La consigna pide analizar:

- Organizacion formal del paper Wu-Lo.
- Claridad de la descripcion de distribucion y recuperacion.
- Notacion y cambios de notacion.
- Si la imagen recuperada es exactamente igual a la original y por que.
- Seguridad de guardar semilla y numero de sombra en bytes 6-9; alternativas.
- Criterio usado para `k != 8`, tamano de portadoras y propuestas descartadas.
- Facilidad de implementacion.
- Extension a imagenes color de 24 bpp.
- Dificultades de lectura/implementacion.
- Extensiones o modificaciones posibles.
- Situaciones de aplicacion.

## Supuestos actuales

- Se trabajara solo con BMP de 8 bpp sin compresion.
- La entrega final se hara en C.
- La fecha de entrega indicada por consigna es 8 de junio de 2026.
- No hay repositorio Git inicializado en esta carpeta al momento de crear este archivo.
