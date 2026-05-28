# Markdown en una página

**Markdown** es una forma de escribir texto con formato usando símbolos
sencillos. Lo que está a la izquierda es lo que tecleas; a la derecha, cómo
se ve. En md-editor no necesitas teclear estos símbolos: los aplicas con la
barra de herramientas y, al guardar, el editor los genera por ti.

## Índice

- [Párrafos y saltos de línea](#parrafos-y-saltos-de-linea)
- [Encabezados](#encabezados)
- [Énfasis](#enfasis)
- [Listas](#listas)
- [Citas](#citas)
- [Código](#codigo)
- [Enlaces e imágenes](#enlaces-e-imagenes)
- [Reglas horizontales](#reglas-horizontales)
- [Tablas](#tablas)
- [Fórmulas matemáticas](#formulas-matematicas)
- [Escapes](#escapes)

## Párrafos y saltos de línea

Separa párrafos con una **línea en blanco**. Dentro de un párrafo, dos
espacios al final de una línea fuerzan un salto sin abrir un párrafo nuevo.

## Encabezados

```
# Título de nivel 1
## Título de nivel 2
### Título de nivel 3
```

Hasta seis niveles (`######`). En md-editor también puedes aplicarlos desde
**Formato → Encabezado** o con Ctrl+1 … Ctrl+6.

## Énfasis

- `*cursiva*` o `_cursiva_` → *cursiva*
- `**negrita**` o `__negrita__` → **negrita**
- `***negrita y cursiva***` → ***negrita y cursiva***
- `~~tachado~~` → ~~tachado~~

## Listas

**Viñetas** (con `-`, `*` o `+`):

```
- Manzana
- Pera
  - Conferencia
  - Ercolini
```

**Numeradas**:

```
1. Primero
2. Segundo
3. Tercero
```

**Tareas** (casillas):

```
- [x] Hecho
- [ ] Por hacer
```

## Citas

Una o más líneas precedidas por `>`:

```
> El que lee mucho y anda mucho, ve mucho y sabe mucho.
> — Miguel de Cervantes
```

## Código

**En línea**: rodea con un acento grave: `` `código` ``.

**Bloque**: tres acentos graves al inicio y al final; opcionalmente, el
nombre del lenguaje para colorearlo:

````
```python
def saluda(nombre):
    print(f"Hola, {nombre}")
```
````

## Enlaces e imágenes

- **Enlace**: `[texto](https://ejemplo.com)`
- **Enlace con título**: `[texto](https://ejemplo.com "Título emergente")`
- **Imagen**: `![texto alternativo](ruta/imagen.png)` — igual que el enlace,
  pero con `!` delante.

En md-editor, **Ctrl+clic** sobre un enlace lo abre en el navegador.

## Reglas horizontales

Tres o más guiones, asteriscos o guiones bajos en una línea propia:

```
---
```

## Tablas

```
| Producto | Cantidad | Precio |
|----------|---------:|:------:|
| Pan      |        2 |  1,20 €|
| Leche    |        1 |  0,95 €|
```

Los dos puntos en la línea de separación marcan la alineación de columna:
`:--` izquierda, `:-:` centro, `--:` derecha. md-editor conserva la
alineación al guardar.

## Fórmulas matemáticas

Markdown estándar **no** define fórmulas, pero hay una convención muy extendida
(Pandoc, Obsidian, Quarto, GitHub) que md-editor admite: la sintaxis de TeX
entre `$...$` (en línea) y `$$...$$` (en bloque).

```
La fórmula $E = mc^2$ es muy famosa.

$$
\sum_{i=1}^n a_i = \frac{n(n+1)}{2}
$$
```

Los caracteres especiales de TeX (`\`, `_`, `*`, `{`, `}`) se conservan
intactos dentro de las fórmulas — el editor los protege para que el parser de
Markdown no los confunda con cursiva o negrita.

En md-editor las fórmulas se ven renderizadas con super y subíndices reales
(no como `$x^2$` literal). Inserta una con **Insertar → Fórmula…**
(Ctrl+Shift+F) o haz doble clic sobre una existente para editarla.

## Escapes

Para que un símbolo de Markdown aparezca literal (sin actuar como formato),
ponle delante una barra invertida: `\*no es cursiva\*` → \*no es cursiva\*.

Los símbolos escapables son:
```
\ ` * _ { } [ ] ( ) # + - . ! |
```
