# Rediseño de los iconos de la barra de formato

Material de estudio, **fuera de la build del editor**: un banco de pruebas que
pinta varias alternativas y las láminas PNG para compararlas. Sirvió para elegir
el diseño que hoy está en `src/widgets/formaticons.cpp`, y queda aquí para poder
volver a ensayar proporciones sin partir de cero.

## Qué fallaba

Los tres iconos de lista comparten el 80 % de su dibujo: tres «líneas de texto» a
la derecha, idénticas en los tres, y un marcador a la izquierda de unos 0,3 × el
lado del icono. En la barra el icono mide 18-22 px reales, así que ese marcador
—lo único que distingue un botón de otro— queda en 5-6 px:

- La lista numerada dibuja «1 2 3» con la fuente a `0,34 × N`, es decir 6-7 px de
  alto, con astas de menos de un píxel. El antialiasing las convierte en tres
  manchas grises que no se leen y que además desentonan con las líneas de al
  lado, que sí van a tinta plena y 2 px de grosor.
- Las tareas dibujan tres casillas de 4-5 px con una marca de verificación
  dentro de la primera. A ese tamaño la marca es un borrón dentro del recuadro.
- Resultado: los tres botones se leen como la misma mancha rayada y hay que ir al
  tooltip para saber cuál es cuál.

De propina, los cuatro iconos de carácter (N/C/S/T) se centran con `AlignCenter`
sobre la caja de línea de la fuente, que incluye el hueco del descendente; como
las cuatro letras son mayúsculas sin descendente, el glifo queda alto dentro del
botón. Y el subrayado de la «S», al ser el de la fuente, se pega al borde
inferior del icono y a 18 px llega a recortarse.

## Alternativas ensayadas

| Variante | Iconos de lista |
| --- | --- |
| `Actual` | lo de hoy, como referencia |
| `Aire` | dos filas en vez de tres; marcador al doble de tamaño; cifras de fuente |
| `Trazo` | tres filas, pero las cifras dibujadas a trazo en vez de con la fuente |
| `Cifra` | dos filas + cifras a trazo + casilla de tarea grande |
| `Check` | igual que `Cifra`, pero las tareas son marcas de verificación sin casilla |

Las dos decisiones que cambian las cosas:

- **Dos filas, no tres.** Es lo que libera altura: el marcador pasa de ~0,22 × N
  a 0,30 × N sin ensanchar el icono. La metáfora de lista se sostiene igual con
  dos renglones.
- **Cifras dibujadas a trazo.** En vez de pedirle a la fuente un «1» de 7 px, se
  dibujan con el mismo grosor de trazo que las líneas de texto. Pierden finura
  tipográfica y ganan peso y silueta, que es lo que se ve a este tamaño.
- **Tareas sin recuadro.** La marca de verificación es lo que significa «tarea»;
  el recuadro solo le roba sitio. Sin él, la marca ocupa el marcador entero y el
  botón de tareas deja de parecerse a los otros dos. Es lo que hacen los juegos
  de iconos modernos (`list-checks` de Lucide, p. ej.).

## Las láminas

En `muestras/`:

- `barra-claro.png` / `barra-oscuro.png` — la maqueta más útil: las cinco
  variantes en el contexto real de la barra, con los botones de texto («H1»,
  «Cita», «Bloque de código») al lado, que es contra lo que compite el peso del
  icono.
- `contacto-claro.png` / `contacto-oscuro.png` — los siete iconos de cada
  variante a 18, 22, 28 y 36 px, tema claro y oscuro.
- `detalle-<variante>.png` — cada juego a 22 px ampliado 8× sin suavizado: enseña
  exactamente qué píxeles se pintan.
- `detalle-propuesta.png` — el mismo detalle, generado con el código que acabó
  aplicándose (comprobación de que lo que hay en `src/` pinta lo que promete la
  variante `Check`).

## Lo que se aplicó

La variante `Check` para las listas, y el centrado óptico para los cuatro de
carácter: es lo que hay hoy en `src/widgets/formaticons.cpp`. `formaticons.h` no
cambió —misma API, mismas tres funciones— ni `MainWindow`: los iconos se siguen
generando en `updateToolBarIcons()` con el color del tema y el tamaño del zoom.

Cambios respecto al diseño anterior, uno a uno:

- **Listas** — dos filas; viñetas más grandes; cifras 1/2 a trazo; tareas con dos
  marcas de verificación sin casilla.
- **Carácter** — centrado por `tightBoundingRect` en vez de `AlignCenter`; cuerpo
  de `0,66 × N` a `0,62 × N` para dejar sitio al subrayado; subrayado pintado a
  mano a distancia fija de la base de la letra.
- **Tachado** — se deja el de la fuente a propósito. Pintarlo a mano (como el
  subrayado) lo alarga más allá de la «T» y el icono acaba pareciendo un «+»;
  está probado y descartado.
- **Tablas** — sin cambios. Los iconos de fila/columna de la barra flotante de
  tablas siguen con la retícula de tres filas; si el aire de dos convence,
  convendría repasarlos para que la familia sea coherente.

## Volver a generar las láminas

```bash
cd Auxiliar/icons
cmake -S . -B build && cmake --build build
QT_QPA_PLATFORM=offscreen ./build/iconpreview muestras
```

`iconvariants.cpp` tiene las cinco variantes parametrizadas en una tabla
(`kAire`, `kTrazo`, `kCifra`, `kCheck`): número de filas, largo de las líneas,
grosor del trazo y tamaño del marcador. Tocar ahí un número y volver a generar es
el ciclo rápido para probar otra proporción.
