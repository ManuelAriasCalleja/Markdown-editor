# Prueba de fórmulas

Texto normal seguido de una fórmula en línea: $E = mc^2$, y otra con
subíndices: $m_0$ kg. Letras griegas: $\alpha + \beta = \gamma$.

## En línea más exigentes

- Sumatorio: $\sum_{i=1}^n a_i$
- Integral: $\int_0^\infty e^{-x} dx$
- Relaciones: $x \leq y \neq z \approx w$
- Conjuntos: $x \in \mathbb{R}$, $A \subseteq B \cup C$
- Lógica: $\forall x \exists y$, $p \land q \Rightarrow r$
- Raíz: $\sqrt{2}$, $\sqrt{x^2 + y^2}$
- Fracción: $\frac{a}{b}$, $\frac{x^2 + 1}{2x}$

## En bloque

Una identidad clásica:

$$E = m c^2$$

La función zeta de Riemann:

$$\zeta(s) = \sum_{n=1}^\infty \frac{1}{n^s}$$

## Caracteres que romperían el round-trip sin protección

Subrayados que **no** son cursiva: $a_b_c$ y $x_1 + x_2 + x_3$.
Asteriscos: $a*b*c$.
Backslashes: $\alpha \to \beta$, $\Rightarrow$ implica.

## Lo que se hereda del documento normal

Esto es **negrita**, esto es *cursiva*, esto es `código en línea` (no
fórmula). Y un enlace [a Wikipedia](https://es.wikipedia.org).

| Variable | Valor          |
|----------|----------------|
| $x$      | $\frac{1}{2}$  |
| $\pi$    | $\approx 3.14$ |

```python
# Esto NO es fórmula (está dentro de un bloque de código)
total = sum(a_i for i in range(n))
```

## Precios y otros usos del `$`

Cuesta $ 5 y otros $ 10. Y \$50 escapado. Nada de esto debe verse como
fórmula.
