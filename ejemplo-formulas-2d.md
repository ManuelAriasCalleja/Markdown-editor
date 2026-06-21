---
title: Prueba de fórmulas en 2D (Nivel 2)
lang: es
---

# Fórmulas en 2D — guía de comprobación

Este documento ejercita la maquetación **2D real**: fracciones apiladas con
barra y grandes operadores con sus límites encima/debajo. Cada sección dice qué
deberías ver. Haz **doble clic** en cualquier fórmula para reabrir el editor, y
prueba el **zoom** (Ctrl+rueda): las fórmulas 2D deben escalar contigo.

## 1. Fracciones apiladas (deben verse con barra horizontal real)

Una fracción simple en bloque:

$$\frac{a + b}{c - d}$$

Numerador y denominador con más estructura:

$$\frac{x^2 + 2x + 1}{x + 1}$$

Fracción **anidada** (una fracción dentro de otra):

$$\frac{1}{1 + \frac{1}{1 + \frac{1}{x}}}$$

En línea, una fracción pequeña $\frac{1}{2}$ dentro del párrafo (queda algo
elevada respecto al texto: es la limitación conocida del alineado inline).

## 2. Grandes operadores con límites (encima y debajo)

Sumatorio con índice inferior y superior:

$$\sum_{i=1}^n i = \frac{n(n+1)}{2}$$

Producto:

$$\prod_{k=1}^n k = n!$$

Integral definida (límites como en LaTeX display):

$$\int_0^\infty e^{-x^2}\,dx = \frac{\sqrt{\pi}}{2}$$

La función zeta, combinando operador grande y fracción en el numerando:

$$\zeta(s) = \sum_{n=1}^\infty \frac{1}{n^s}$$

## 3. Casos clásicos combinados

Fórmula cuadrática (fracción + raíz):

$$x = \frac{-b \pm \sqrt{b^2 - 4ac}}{2a}$$

Media de los cuadrados (operador grande dentro del numerador):

$$\bar{x} = \frac{\sum_{i=1}^n x_i^2}{n}$$

Definición de la derivada:

$$f'(x) = \lim_{h \to 0} \frac{f(x + h) - f(x)}{h}$$

## 4. Fórmula `$$` multilínea en la fuente

Esto se escribe en varias líneas en el Markdown y debe cargarse como **una**
sola fórmula 2D:

$$
\frac{\partial}{\partial t} \Psi
=
\frac{i \hbar}{2m} \nabla^2 \Psi
$$

## 5. Lo que sigue en línea (NO necesita 2D)

Estas se componen como antes (super/subíndice de Qt, sin apilar):

- Potencias y subíndices: $E = mc^2$, $x_1 + x_2$, $a_i^{2}$.
- Griego y operadores: $\alpha + \beta = \gamma$, $x \leq y \neq z$.
- Conjuntos: $x \in \mathbb{R}$, $A \subseteq B \cup C$.
- Un sumatorio **sin** límites tampoco se apila: $\sum a_i$.

## 6. Comprobación de round-trip y export

Guarda el archivo (Ctrl+S) y vuelve a abrirlo: todas las fórmulas deben
sobrevivir intactas. Exporta a HTML/PDF/ODT/DOCX (*Archivo → Exportar*): las 2D
se vuelcan a su aproximación en línea; a LaTeX (*.tex*) salen como `$$...$$`
literales.

| Magnitud   | Expresión                       |
|------------|---------------------------------|
| Área       | $\pi r^2$                       |
| Gaussiana  | $\frac{1}{\sigma\sqrt{2\pi}}$   |
| Binomio    | $\sum_{k=0}^n \binom{n}{k} x^k$ |

```text
$$\frac{a}{b}$$  ← esto está dentro de un bloque de código: NO es fórmula.
```
