Secreto Compartido  

Lic. Ana María Arias Roig  

Introducción  

Adi Shamir y George Blakley conciben en 1979, aunque en forma separada, el concepto de Secreto Compartido como una manera de proteger claves.  

Shamir, en su trabajo, justifica la creación del método, diciendo que  

“Para proteger los datos, los encriptamos, pero para proteger la clave de  encripción necesitamos un método diferente”.  

Plantea que guardar la clave en un solo lugar es altamente riesgoso y guardar  múltiples copias en diferentes lugares sólo aumenta la brecha de seguridad. Concluye,  entonces, que el secreto (D) deberá dividirse en un número fijo de partes (D1, D2, ..., Dn) de  forma tal que:  

1\. Conociendo un subconjunto de k cualesquiera de esas partes se pueda reconstruir  D. 

2\. Conociendo un subconjunto de k-1 cualesquiera de esas partes el valor D quede  indeterminado. 

En el mismo documento explica un método basado en interpolación polinómica que  considera muy robusto para cumplir los objetivos anteriores:  

“Con n \= 2k-1, obtenemos un esquema muy seguro de administración de la  clave: podemos recuperar la clave aún cuando \[n/2\] \= k-1 de las n piezas  hayan sido destruidas, mientras que los oponentes no pueden recuperar la  clave aún cuando las brechas de seguridad expongan \[n/2\] \= k – 1 de las  piezas restantes”  

Por su lado, Blakley considera también la cuestión de la protección de claves diciendo  que, frente a la importancia de determinadas claves criptográficas, se presenta el dilema de  hacer muchas copias y correr el riesgo de que algunas se extravien o hacer pocas copias  corriendo el riesgo de que se destruyan todas. Contempla tres tipos de incidentes de los  cuales habrá que proteger a las piezas de información:  

a) Abnegation – Repudio: tras este incidente, una pieza de información ya no  se puede reclamar a la persona a la que se había confiado en custodia.  Comprende casos de destrucción, degradación y deserción.  

b) Betrayal – Traición: tras este incidente, la pieza de información es  completamente conocida por el oponente. Comprende casos de deserción y abandono.  

c) Combination \- Combinación: es un incidente que combina un repudio con  una traición.  

Estos tres tipos de incidentes (A, B, C) generan un total d \= a \+ b – c de incidentes  que se espera que puedan ocurrir. Dado que se debe anticipar el máximo número de  incidentes que puede ocurrir, que es a \+ b, el número de guardianes debería ser de g \= a \+ b  \+ 1\. Por lo tanto, la sugerencia de Blakley es que la clave se distribuya en a \+ b \+ 1 piezas a  las que denomina sombras. La clave debe ser reconstruible a partir de cualquier conjunto de  b \+ 1 sombras, a la vez que con b de dichas sombras no se pueda obtener ningún tipo de  información.  

El documento de Blakley describe una forma de lograr el objetivo de distribuir las  sombras de la manera exigida, utilizando conceptos de geometría proyectiva.  

Definiciones relativas a esquemas de secreto compartido  

Con el fin de unificar el vocabulario y facilitar la comprensión del tema propuesto se exponen  las siguientes definiciones:   
• S.S.S. Secret Sharing Scheme: Esquema de Secreto Compartido. Es un método por el  cual n piezas de información llamadas sombras (shares o shadows) son asignadas a  una clave secreta ( k ) de manera tal que:  

1\. k puede ser reconstruida a partir de ciertos grupos autorizados de sombras.  2\. k no puede ser reconstruida a partir de los grupos no autorizados de sombras.  s ,s ,...,s 1 2:    
En términos de entropía, si las sombras son n 

1\. ( | , ,..., ) 0   
i i i H k s s s .    
1 2\= t 

2\. H(k s s s ) H(k )   
| , ,..., .    
i i i \= 1 2 −1   
t 

• Espacio de claves: La clave se selecciona de un conjunto finito Κ de posibles claves,  que es de público conocimiento. ( k ∈ Κ ).En general, el valor de k lo elige un  participante confiable, el dealer o distribuidor (D). 

• Espacio de sombras: Es el conjunto S de todas las posibles sombras. El esquema de  reparto del secreto permite obtener un conjunto de piezas de información, las  sombras, que luego se repartirán a los participantes. Las piezas deben distribuirse de  manera secreta.  

• Esquema Perfecto: El esquema es perfecto si los grupos no autorizados de  participantes, al reunir sus sombras no pueden obtener ningún grado de información  acerca del secreto k .Por ejemplo, un esquema de secreto compartido podría ser,  para la clave “password” dividirla en las sombras: “pa------”, “—ss----””----wo--””----- 

\-rd”. La concurrencia de las cuatro sombras permite obtener la clave. Sin embargo,  poseer una sombra ya brinda algo de información. Y poseer dos sombras puede dar  información suficiente como para adivinar la clave. Este no es un esquema perfecto.  

• Participantes: Son los que reciben las sombras. El dealer D no pertenece a este  conjunto. Si llamamos ℘ al conjunto de participantes, entonces su cardinal es n. Es  decir: ℘= {P1, P2,..., Pn } \= {Pi 1/ ≤ i ≤ n} con \#℘= n . El subconjunto S S i ⊆ representa el conjunto de posibles sombras que el participante Pi podría recibir.  • Estructura de acceso o Esquema de Concurrencia: De los n   
2 subconjuntos de ℘,  

no todos estarán autorizados para reconstruir el secreto. La estructura de acceso Γ está formada por aquellos subconjuntos autorizados de participantes. Ejemplo: Si  ℘= {P1, P2,P3,P4 }, entonces n vale 4 y hay 16 subconjuntos de ℘. Una posible  estructura de acceso podría ser Γ \= {{P1, P2,P3 }{P1, P4 },{P3, P4 }} y otra podría ser  Γ \= {{P1, P2, P3 },{P1,P2, P4 },{P1, P3, P4 },{P2, P3, P4 }} Cada participante debe  aparecer por lo menos una vez en alguno de los subconjuntos del esquema.  

• Estructura de acceso monótona: La estructura es monótona si, dados B y C subconjuntos de ℘, ocurre que siempre que B ∈ Γ y B ⊆ C , entonces C ∈ Γ . Es  decir, si un conjunto de ℘ pertenece a la estructura de acceso, también pertenece  todo conjunto que lo contenga. Para el conjunto ℘ del ejemplo anterior, una  estructura monótona podría ser: Γ \= {{P1, P2, P3, P4 },{P1, P2,P3 }}, porque  {P1,P2, P3 } sólo está contenido en {P1,P2, P3, P4 }, que también pertenece a la  estructura de acceso.  

• Base de la Estructura de acceso: Es el subconjunto de la estructura de acceso cuyos  elementos son los subconjuntos autorizados minimales. Es decir, está formada por  aquellos conjuntos de Γ cuyos subconjuntos ya no pertenecen a Γ .   
Γ0 \= {X ∈ Γ / X es minimal} 

Es decir:  

Γ0 \= {X ∈ Γ /∀Y ⊂ X :Y ∉ Γ} 

En el ejemplo anterior, Γ0 \= {{P1, P2,P3 }} 

• Rango de una estructura de acceso: Es la máxima cardinalidad que presentan los  subconjuntos minimales que conforman la base. En el caso anterior, es 3\. 

• Esquema de Umbral o estructura de acceso umbral: Un esquema de umbral ( , nt ) con t ≤ n es un método para compartir una clave k entre un número finito de n participantes de manera tal que con un subconjunto cualquiera de t participantes o  más se pueda reconstruir el valor de k , pero ningún grupo de t −1 participantes  pueda hacerlo.  

Es decir:  

Γ \= {A ⊂℘/\# A ≥ t}.  

En un esquema de umbral, la Base de la estructura de acceso consiste de todos los  subconjuntos de exactamente t participantes. El rango, por lo tanto, es t .  

Para el conjunto ℘= {P1, P2,P3,P4 } de participantes, el esquema:  

Γ \= {{P1, P2, P3 },{P1,P2, P4 },{P1, P2,P3,P4 }} 

no es de umbral, porque hay grupos de 3 participantes que no están autorizados.  Mientras que el esquema:  

Γ \= {{P1, P2, P3 },{P1, P2, P4 },{P1, P3, P4 },{P2, P3, P4 },{P1, P2,P3,P4 }} sí es una estructura de acceso de umbral 3, ya que cualquier subconjunto de ℘ de 3  o más participantes está autorizado para reconstruir el secreto. Es un esquema de  umbral (3, 4\)  

• Tasa de información de un SSS: La clave puede representarse mediante una cadena  de bits de longitud k 2   
log . Si el participante Pi recibe el conjunto i S de sombras,  

entonces la información del participante puede medirse como 2 Si   
log . Por lo tanto  

la tasa de información para el participante Pi es la razón:  

ρ \=   
log   
2   
k 

iS   
log   
2   
i 

y para el SSS es:  

ρ \= min{ρ i 1/ ≤ i ≤ n} 

mientras que la tasa de información promedio de un SSS es:  n k   
ρ   
\= n ∑  
log log   
2 2   
S 

i   
i   
\=   
1 

En un esquema de Secreto Compartido Perfecto, ρ ≤ ρ ≤ 1. Esto significa que el  tamaño de la clave no puede ser superior al tamaño de la sombra.   
• Esquema de Secreto Compartido Ideal: Aquél en que ρ \= ρ \= 1. Los mejores  esquemas son aquéllos en los que los fragmentos tienen casi la misma longitud que el  secreto.  

Esquemas de Umbral.  

Los primeros esquemas de secreto compartido de Shamir y de Blakley fueron de umbral. Luego hubo más esquemas de umbral que no tienen un aspecto geométrico (por  ejemplo el que usa el teorema chino del resto).  

Esquema de umbral (t, n) de Shamir.  

Dados t puntos en un plano bidimensional ( ) ( ) ( ) t t   
x , y , x , y ,..., x , y 1 1 2 2 con distintas    
x , hay uno y sólo un polinomio (xq ) de grado t −1 que contiene a todos esos    
coordenadas i   
puntos. Es decir, un solo polinomio (xq ) es tal que ( ) i i   
y \= xq , para todos los i 

considerados. Teniendo en cuenta esto, para un secreto S , se elige un polinomio al azar de  grado t −1, con t \< n en el cual el término independiente 0 a es igual al secreto S :  ( )1   
\= \+ \+ \+ \+ −t   
1 2...−   
2   
xq S xa a x a x   
t   
1 

Se evalúan las n sombras S q( ) S q( ) S (nq ) 1 \= ,1 2 \= 2 ,..., n \= y se distribuyen a los  participantes.  

Para reconstruir el secreto S , deberá reunirse un suconjunto t de pares ( )i   
, Si .  

Mediante interpolación, o por la resolución del sistema de t ecuaciones lineales  correspondiente se podrán obtener los coeficientes de (xq ) y obtener finalmente  q( ) \= a \= S 0 0 

Un esquema de Shamir (2, 3\) se representaría:  

S   
P1 

P2 

P3 

Es evidente que, con menos de 2 puntos no se puede obtener el polinomio (en este  caso una recta) y por lo tanto no se puede hallar el valor de S.  

Para su implementación, Shamir propone trabajar en un cuerpo finito: Ζp. El número  primo elegido deberá ser p \> S ∧ p \> n .  

,..., t− a a , para el polinomio (xq ) se eligen dentro de Ζp − {0}1. Es    
Los coeficientes 1 1 

decir 1 ≤ a ≤ p −1 i, ∀i 1: ≤ i ≤ t −1. Luego se calculan, módulo p , los valores S (iq ) i \= 

    
a a en (xq ) se eligen aleatoriamente de una    
1 Shamir afirma que “los coeficientes 1   
,..., 1 t− 

distribución sobre los enteros en \[ ,0 p) ”, es decir incluye claramente al 0\. Sin embargo, el coeficiente    
∀i 1: ≤ i ≤ n que se entregan a los participantes. Un grupo de t participantes con sus  respectivas sombras pueden obtener el secreto resolviendo el sistema de t ecuaciones  linealmente independientes2:  

t   
−   
1   
xa ( ) xq y ( ) j t   
i ∑ ji \= \= ≤ ≤   
i   
\=   
0   
j j   
1, 

Es decir:     
1 

2 

t   
−   
1   
a xa a x a x y ...   
\+ \+ \+ \+ \=   
0 11 1   
12 2 

t   
−   
11 t   
−   
1 

1   
a xa a x a x y ...   
\+ \+ \+ \+ \= 

   
0 21 ...   
22 

t   
−   
21   
2 

1   
2   
t   
−   
1   
a xa a x a x y ...   
\+ \+ \+ \+ \=   
0 1   
t t t t   
2   
−   
1   
t 

Shamir propone usar la fórmula de interpolación de Lagrange para obtener los coeficientes:  

t 

x x   
( ) ∑ ∏ \= ≤≤ ≠ −−   
k   
xq y   
\=   
tx x   
1 1 ,   
j jktk j k 

Como sólo interesa a \= q(0) \= S 0, lo que hay que calcular es:  

t   
x   
∑ ∏ \= ≤≤ ≠ −   
y   
k   
tx x   
j jktk k j   
1 1 , 

Es decir, S resulta una combinación lineal de las sombras.  

Stinson propone resolver el sistema de ecuaciones teniendo en cuenta que el mismo  se corresponde, en términos de matrices, con:  

1   
2 

t   
− 

1   
   
a   
0 

   
y   
−   
1 ...   
x x x   
   
   
1   
1 1   
1   
2   
1 t   
− 

1   
a   
1 

y   
1 ...   
x x x   
2   
2   
2   
2 

a   
\=   
... ... ... ... ... 

...   
2   
... 

t   
−   
1 2 1 1 ...   
x x x   
t t t 

a 

t   
t 

y   
1 

Y como la matriz de coeficientes ( )ji X \= x es de Vandermonde su determinante es:  ∏( )   
∆ \= −   
x x 

1   
≤\<≤ tjk   
j k 

que es distinto de 0 (ya que x x j k j ≠ k∀ ,∀ ) lo cual asegura que este sistema tenga  una solución única en el cuerpo Ζp 

Si un grupo de t −1 participantes con sus sombras intentan obtener el secreto,  deberán resolver un sistema de t −1 ecuaciones con t incógnitas, por lo cual la solución es  


de t−1 a no puede ser 0 ya que en ese caso el polinomio dejaría de ser de grado t −1, para ser de un  grado menor.    
2 El sistema de ecuaciones resulta linealmente independiente porque se forma una matriz de  Vandemonde cuyos valores de la segunda columna son todos distintos.   
indeterminada. Los t −1 participantes no pueden descubrir el secreto. En el caso de que se  considere un valor al azar para ( ) 0 0 a \= q 0 \= y , el nuevo sistema de ecuaciones:  

1 0 0 0 0   
a   
y   
− 

1 

2 

t   
−   
1   
 

0   
   
 

0   
   
1 ...   
x x x   
a   
y   
1 1   
1   
2   
1 t   
−   
1   
1   
1 

1 ...   
x x x   
a   
\=   
y   
2   
2   
2   
2   
2 

... ... ... ... ... 1 2 1   
... ... 

t   
−   
1 ...   
x x x   
a   
y   
t t t   
t t 1 

Por tener también una matriz de Vandermonde con coeficientes x x j k j ≠ k∀ ,∀ , será  de solución única. Es decir, para cada valor 0 a existe un único polinomio posible. En  consecuencia, hay tantos polinomios posibles como valores se puedan elegir en Ζp. Todas las  claves del espacio de claves son equiprobables, lo que asegura que el esquema sea perfecto.  

Esquema de umbral (t, n) de Blakley.  

La solución de Blakley usa geometría proyectiva finita.  

Blakley describe en su documento una forma de distribuir la clave en a \+ b \+13 sombras, de manera que pueda reconstruirse a partir de cualquier grupo de b \+14 de esas  sombras, pero que con b de ellos no se pueda obtener nada.  

Previo al desarrollo de su esquema describe una serie de lemas y teoremas que le  sirven para demostrar que el esquema es seguro.  

La idea básica del esquema es la de considerar un espacio vectorial V de dimensión  b \+ 2 , sobre un cuerpo finito Z p donde p es un número primo. Es decir que ( \+2) \=b V Z p. A  cada uno de los (a \+ b \+1) participantes (él los llama guardianes) se les asigna un subespacio  de V de dimensión b \+1:V(g) ⊂ V / dimV(g) \= b \+1. Cuando b \+1 participantes  intersectan sus sombras, es decir sus subespacios, el resultado es una recta (kL ). En realidad  Blakley, considera a la recta como una clase de equivalencia, (es decir toma un punto  proyectivo) cuyas coordenadas son una (b \+ 2)\-upla de valores en Z p, uno de los cuales es  la clave k .  

Traducido a términos de espacio proyectivo, diremos que se toma un espacio  proyectivo finito de dimensión b \+1, y que cuando b \+1 hiperplanos de dimensión b se  intersectan lo hacen en un punto, una de cuyas coordenadas es la clave k .  

Lo primero que hay que hacer en este esquema es determinar los valores a y b y un  valor Q suficientemente pequeño que será una medida del punto de partida para una completa aleatoriedad del procedimiento. Luego elegir un número primo p , impar, que  satisfaga la inecuación que surge de uno de los lemas, que le permite asegurar una  distribución adecuada de valores en la matriz:  2   
0 2((a b 2)(b 1) 1) 2 pQ 2 p ((a b 2)(b 1) 1)p   
\< \+ \+ \+ − \< \< \< \+ \+ \+ − .  

Para construir la matriz M con (a \+ b \+ 2) filas y (b \+ 2) columnas se coloca  primero, en cada fila, una entrada en 1\. En la primera fila, en una entrada al azar se guarda  k ∈ . Las restantes (a \+ b \+ 2)⋅(b \+1)−1 entradas se eligen al azar de Z p.    
la clave Z p 

    
3a \= cantidad de incidentes de abnegation (repudio) b \= cantidad de incidentes de betrayal (traición)  4 por si se da el peor de los casos: los b incidentes de traición.   
Ejemplo: a \= ,1 b \= ,1 Q \= ,0 0001, p \= 491 220 447 1   
41 2 1   
   
182 1 287 

1 241 171 

Para aceptar la matriz debe cumplirse que:  1\) haya un sólo 1 en cada fila  

2\) no haya ningún 0  

3\) no haya dos entradas iguales (a menos que sean las colocadas en 1\)  4\) no debe tener ninguna submatriz (b \+1)⋅ (b \+1) cuyo determinante sea 0\.  

5\) no debe tener ningun par de submatrices (b \+1)⋅ (b \+1) cuyos determinantes  sean iguales.  

Por cómo fueron elegidos los valores, la probabilidad de que la primera matriz que se  produzca cumpla esas condiciones es alta.  

Con dicha matriz se forman (a \+ b \+1) submatrices b \+1 filas que contengan la  primera fila de M :  

220 447 1   
( )    
  1   
   
\=182 1 287 220 447 1   
( )    
  2   
   
\=1 241 171 220 447 1   
( )    
  3   
   
\=41 2 1 

Cada conjunto es linealmente independientes ya que cada submatriz (b \+1)⋅ (b \+1) es no inversible (su determinante es distinto de 0).  

Sea  ( j) la submatriz (b \+1)⋅ (b \+ 2) formada a partir del j − ésimo de estos  (a \+ b \+1) conjuntos de filas.  

Si a la matriz  ( j) se le agrega una última fila x , resulta una matriz Y( xj )r, , de  r r es un subespacio vectorial de    
(b \+ 2)⋅ (b \+ 2). El conjunto U( j) \= {x ∈V / detY( , xj ) \= 0} 

dimensión b \+1, al que pertenece la primer fila de M , que es la primera y última fila de  Y( xj )r, ∀j .  

En nuestro ejemplo:    
r r   
U(1) \= {x ∈V / detY( ,1 x) \= 0} 

Y xr   
220 447 1 

( ) 182 1 287 0 det ,1 0   
\= ⇔ \= 

x x x   
1 2 3 

447 1   
220 1   
220 447   
⇔ x1⋅ − x2⋅ \+ x3⋅ \=0   
1 287   
182 27   
182 1   
137 381 372 0 ⇔ x1 \+ x2 \+ x3 \=   
r r   
U(1) \= {x ∈V / detY( ,1 x) \= 0} \= {x ∈V /137x1 \+ 381x2 \+ 372x3 \= 0}   
r r   
U(2) \= {x ∈V / detY( ,2 x) \= 0} \= {x ∈V / 91x1 \+188x2 \+ 36x3 \= 0}   
r r   
U(3) \= {x ∈V / detY( ,3 x) \= 0}\= {x ∈V / 445x1 \+ 312x2 \+ 280x3 \= 0}   
r es una ecuación lineal de la forma    
La ecuación detY( , xj ) \= 0 

( ) ... 0 Cjx11 \+ Cjx22 \+ \+ C bj \+2xb\+2 \= , donde cada Cji es el determinante de la matriz de  tamaño (b \+1)⋅ (b \+1) que resulta de eliminar de Y( xj )r, la fila b \+ 2 y la columna i . Esos  determinantes ya vimos, por la forma en que se construyó la matriz, que son distintos de 0 y  distintos de a pares, por lo que se obtiene una solución.  

Cuando b de esos subespacios se intersectan, se resuelven simultáneamente b ecuaciones, de las que se obtiene un subespacio de dimensión 2 que no aporta información  sobre cómo recuperar la clave.  

Pero cuando b \+1 de estos subespacios se intersecta, su intersección es la línea a  través del origen que también contiene a la primer fila de M , donde está k . Una base para  r, que es algún múltiplo no nulo de la primer fila    
esta línea es el vector ( ) 1 2 2   
, ,..., \= b\+ g g g g   
de M . Si se conoce gr, para cada entrada i g se puede hallar el valor    
r r r   
h Z hg ( p) i ∈ p/ii ≡ 1 mod . Los b \+ 2 vectores gh h g h g b   
, ,...., \+ son los únicos múltiplos    
1 2 2   
de gr que tienen alguna entrada igual a 1, por lo tanto la primer fila de M , donde está k está entre ellos. Así que una de las (b \+ 2)⋅ (b \+1) entradas distintas de 1, de los  vectores es la clave.  

En el ejemplo, la intersección de dos subespacios permite obtener la solución  S \= {(220x3,447x3, x3)}, que contiene a (220,447 1, ) que es donde está la clave k \= 447 .  

Como se ve, la solución es un punto proyectivo, y en la coordenada de uno de sus  representantes está la clave.  

Dualidad en espacios proyectivos.  

El espacio proyectivo dual de un espacio proyectivo Ρ(E) es el espacio proyectivo Ρ(E \*)deducido del espacio vectorial dual de E 

| PRINCIPIO DE DUALIDAD:   A todo teorema en Ρ(E) que relacione puntos e hiperplanos y esté basado tan sólo  en propiedades de intersección o suma de variedades proyectivas de Ρ(E), le corresponde  un teorema en el espacio proyectivo Ρ(E\*) , llamado teorema dual del anterior, cuyo  enunciado se obtendrá simplemente permutando las palabras “punto” por “hiperplano” e  “intersección” por “suma” y recíprocamente.   Un teorema T relativo a Ρ(E) es cierto si y sólo si también lo es su teorema dual  T \* |
| :---- |

Ejemplo: 

| Proposición o teorema  | Proposición o teorema dual  |
| ----- | ----- |
|  “Los puntos P Q,, R pertenecen a una  misma recta” (están alineados)  | “Las rectas p\*,q\*,r \* se intersectan en el  mismo punto” (son concurrentes). Un haz de  rectas es el dual de una serie de puntos.  |

| P\*   Q\*   R\*  | q\*   p\*   r\*  |
| ----- | ----- |
| “La suma de puntos distintos en el plano  proyectivo es una y sólo una recta”   o bien:   “Dados dos puntos distintos existe una única  recta que los contiene a ambos”   P\*   Q\*  | “La intersección de rectas distintas en el  plano proyectivo es uno y sólo un punto”   o, lo que es equivalente:   “Dadas dos rectas distintas existe un único punto que está contenido en ambas”.   q\*  p\*   |
| “Dados tres puntos distintos en 3  Ρ , existe un   único plano que los contiene a los tres”  | “Dados tres planos distintos en 3  Ρ , existe un   único punto que está contenido en los tres”  |
| “Dados n puntos distintos en n  Ρ , existe un   único hiperplano que los contiene a todos”  | “Dados n hiperplanos distintos en n  Ρ ,   existe un único punto que está contenido en todos”  |

La importancia del principio de dualidad está en que permite duplicar toda la  geometría al dar, para cada teorema, otro igualmente verdadero: su dual. Demostrar un  teorema implica haber demostrado también su dual.  

Si observamos el los últimos tres teoremas del cuadro, son válidos en el espacio  proyectivo pero no en el espacio afín. Dos rectas distintas, en el espacio afín, no siempre se  intersectan en un punto, ya que las rectas que son paralelas no se intersectan nunca. La  ampliación lograda mediante la geometría proyectiva permite que dos rectas distintas  siempre se intersecten en un punto y que su dual también sea cierto.  

De las proposiciones anteriores, la que dice “Dados dos puntos distintos existe una  única recta que los contiene a ambos” se corresponde con un esquema de umbral ( ,2 n) de  Shamir, ya que en dicho esquema se necesitan 2 puntos para obtener un polinomio de grado  1, que sería una recta.  

Por otro lado, su proposición dual “Dadas dos rectas distintas existe un único punto que está contenido en ambas” se corresponde con un esquema de umbral ( ,2 n) de Blakley,   
ya que en dicho esquema se necesitan 2 hiperplanos de dimensión 1 para obtener un punto  proyectivo, en cuyas coordenadas está el secreto.  

Entonces se tiene la siguiente relación de dualidad:  

| “Dados dos puntos distintos existe una única  recta que los contiene a ambos”  | “Dadas dos rectas distintas existe un único punto que está contenido en ambas”.  |
| ----- | ----- |
| Esquema de secreto umbral ( ,2 n) de Shamir  | Esquema de secreto umbral ( ,2 n) de Blakley  |

Sin embargo, un esquema umbral ( , nt ) de Balkley con t \> 2 ya no es dual de Shamir,  aunque constituye otro esquema de secreto compartido.  

Bibliografía  

• Shamir, Adi (1979) “How to share a secret”. Communications of the ACM 22(11): 612- 613\.  

• Blakley, G. R. (1979). “Safeguarding cryptographic keys”. Proceedings of the National  Computer Conference 48: 313 – 317  

• Capítulo 15 de Computer Security – Art and Science, Matt Bishop, Addison-Wesley,  2004  

• Capítulo 10 y 12 de Handbook of Applied Cryptography, Alfred J. Menezes, Paul C.  Van Oorschot, Scott A. Vanstone, CRC Press, 1997 