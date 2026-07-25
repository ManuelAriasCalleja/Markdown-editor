# Ficheros .docx de prueba

Entradas de la mitad de **importación** de `tests/tst_docx.cpp`. Son .docx
*ajenos*: llevan a propósito construcciones que la app nunca genera al exportar,
que son justo las que rompen a un importador. Exportar y volver a importar lo
nuestro no probaría nada de eso.

Se regeneran con:

```bash
python3 scripts/make-docx-fixtures.py
```

| Fichero | Qué ejercita |
|---|---|
| `pandoc-basico.docx` | Un .docx «de otra herramienta», con la estructura canónica: encabezados, marcas de carácter, listas anidadas, cita, bloque de código, tabla con alineaciones, enlace y regla. Generado por Pandoc a partir de `pandoc-basico.md` (que se conserva al lado como referencia de lo que debería volver). |
| `word-tipico.docx` | Imita la salida de Microsoft Word: estilos referenciados por **nombre** (`heading 1`, `Source Code`, `Verbatim Char`), runs partidos **a mitad de palabra** («Negociación» en cuatro runs, como hace Word con el estado de revisión), hipervínculo por **relación** (`w:hyperlink r:id`, no por campo), listas con `numbering.xml`, tabla con fila de encabezado e imagen embebida. |
| `word-raro.docx` | Casos adversarios: **cambios controlados** (`w:ins` se queda, `w:delText` NO debe importarse), ruido que Word intercala (`bookmarkStart`, `proofErr`, `lastRenderedPageBreak`), metacaracteres XML escapados, emoji **fuera del BMP** y acentos combinantes, tabulador y salto de línea suave dentro de un run, párrafos vacíos, **nota al pie** (`word/footnotes.xml`) y **tabla anidada** dentro de una celda. |
| `vacio.docx` | Paquete válido pero sin contenido. La importación no debe producir nada: es la condición que `MainWindow::importWithPandoc` avisa como «el archivo no produjo ningún contenido». |

Son ZIP de XML construidos a mano (`zipfile` + XML literal en el script), sin
dependencias de Python más allá de la biblioteca estándar; el PNG embebido de
`word-tipico.docx` también se genera a mano (`zlib` + `struct`), sin PIL.
`pandoc-basico.docx` es el único que necesita Pandoc instalado para regenerarse;
si falta, el script avisa y conserva el que ya hubiera.

Las pruebas que los usan se **omiten con `QSKIP`** si Pandoc no está instalado
(la importación lo ejecuta por `QProcess`, igual que `MainWindow`), pero
`fixtureFilesAreRealPackages` sí comprueba siempre que los ficheros estén y sean
paquetes legibles.

La extracción de imágenes (`--extract-media`) se prueba **siempre contra un
directorio temporal**, nunca el de aquí: en la app las imágenes se extraen junto al
documento de origen, y las pruebas no deben dejar una carpeta `word-tipico-media/`
en el árbol fuente.
