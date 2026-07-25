#!/usr/bin/env python3
"""Genera los .docx de prueba de tests/fixtures/docx/.

Los .docx son ZIP de XML, así que se construyen aquí a mano (zipfile + XML
literal) para tener control exacto sobre lo que contienen: los ficheros que
produce un procesador de textos real traen construcciones que nuestro
importador (Pandoc) tiene que digerir y que no sabríamos provocar generando el
.docx con la propia app.

Se generan cuatro:

  pandoc-basico.docx  producido por Pandoc a partir de Markdown: un DOCX «de otra
                      herramienta», con la estructura canónica.
  word-tipico.docx    imita la salida de Microsoft Word: estilos por nombre,
                      runs partidos a mitad de palabra, hipervínculo por relación,
                      listas con numbering.xml, tabla e imagen embebida.
  word-raro.docx      casos adversarios: cambios controlados (w:ins/w:del),
                      ruido de Word (bookmarks, proofErr, lastRenderedPageBreak),
                      metacaracteres XML, emoji fuera del BMP, tabla anidada,
                      nota al pie, tabulador y salto de línea.
  vacio.docx          válido pero sin contenido: la importación no debe producir
                      nada (el aviso «no produjo ningún contenido» de MainWindow).

Uso:  python3 scripts/make-docx-fixtures.py
"""

import os
import struct
import subprocess
import sys
import zlib
import zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
OUT_DIR = os.path.join(os.path.dirname(HERE), "tests", "fixtures", "docx")

W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main"
R = "http://schemas.openxmlformats.org/officeDocument/2006/relationships"
WP = "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"
A = "http://schemas.openxmlformats.org/drawingml/2006/main"
PIC = "http://schemas.openxmlformats.org/drawingml/2006/picture"

DOC_NS = (
    f' xmlns:w="{W}" xmlns:r="{R}" xmlns:wp="{WP}" xmlns:a="{A}" xmlns:pic="{PIC}"'
)

XML_DECL = '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'

SECT_PR = (
    '<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
    '<w:pgMar w:top="1417" w:right="1701" w:bottom="1417" w:left="1701"'
    ' w:header="708" w:footer="708" w:gutter="0"/></w:sectPr>'
)


# --- Piezas comunes del paquete ---------------------------------------------

def content_types(extra_overrides=(), png=False):
    defaults = (
        '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
        '<Default Extension="xml" ContentType="application/xml"/>'
    )
    if png:
        defaults += '<Default Extension="png" ContentType="image/png"/>'
    base = (
        '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>'
        '<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>'
        '<Override PartName="/docProps/core.xml" ContentType="application/vnd.openxmlformats-package.core-properties+xml"/>'
    )
    return (
        XML_DECL
        + '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
        + defaults
        + base
        + "".join(extra_overrides)
        + "</Types>\n"
    )


ROOT_RELS = (
    XML_DECL
    + '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
    f'<Relationship Id="rId1" Type="{R}/officeDocument" Target="word/document.xml"/>'
    '<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties" Target="docProps/core.xml"/>'
    "</Relationships>\n"
)


def core_xml(title):
    return (
        XML_DECL
        + '<cp:coreProperties xmlns:cp="http://schemas.openxmlformats.org/package/2006/metadata/core-properties"'
        ' xmlns:dc="http://purl.org/dc/elements/1.1/">'
        f"<dc:title>{title}</dc:title><dc:language>es-ES</dc:language>"
        "</cp:coreProperties>\n"
    )


def styles_xml():
    """Estilos con el `w:name` que usa Word: Pandoc mapea por NOMBRE, no por id."""
    heads = ""
    for n in range(1, 4):
        heads += (
            f'<w:style w:type="paragraph" w:styleId="Heading{n}">'
            f'<w:name w:val="heading {n}"/><w:basedOn w:val="Normal"/>'
            f'<w:next w:val="Normal"/>'
            f'<w:pPr><w:outlineLvl w:val="{n - 1}"/></w:pPr>'
            f'<w:rPr><w:b/><w:sz w:val="{34 - 4 * n}"/></w:rPr></w:style>'
        )
    return (
        XML_DECL
        + f'<w:styles xmlns:w="{W}">'
        '<w:docDefaults><w:rPrDefault><w:rPr>'
        '<w:rFonts w:ascii="Calibri" w:hAnsi="Calibri"/><w:sz w:val="22"/>'
        '<w:lang w:val="es-ES"/></w:rPr></w:rPrDefault></w:docDefaults>'
        '<w:style w:type="paragraph" w:default="1" w:styleId="Normal">'
        '<w:name w:val="Normal"/></w:style>'
        '<w:style w:type="paragraph" w:styleId="Title"><w:name w:val="Title"/>'
        '<w:basedOn w:val="Normal"/><w:rPr><w:b/><w:sz w:val="56"/></w:rPr></w:style>'
        + heads
        + '<w:style w:type="paragraph" w:styleId="ListParagraph">'
        '<w:name w:val="List Paragraph"/><w:basedOn w:val="Normal"/></w:style>'
        '<w:style w:type="paragraph" w:styleId="Quote"><w:name w:val="Quote"/>'
        '<w:basedOn w:val="Normal"/><w:pPr><w:ind w:left="720"/></w:pPr></w:style>'
        '<w:style w:type="paragraph" w:styleId="SourceCode">'
        '<w:name w:val="Source Code"/><w:basedOn w:val="Normal"/>'
        '<w:rPr><w:rFonts w:ascii="Consolas" w:hAnsi="Consolas"/></w:rPr></w:style>'
        '<w:style w:type="character" w:styleId="VerbatimChar">'
        '<w:name w:val="Verbatim Char"/>'
        '<w:rPr><w:rFonts w:ascii="Consolas" w:hAnsi="Consolas"/></w:rPr></w:style>'
        '<w:style w:type="character" w:styleId="Hyperlink"><w:name w:val="Hyperlink"/>'
        '<w:rPr><w:color w:val="0563C1"/><w:u w:val="single"/></w:rPr></w:style>'
        '<w:style w:type="paragraph" w:styleId="FootnoteText">'
        '<w:name w:val="footnote text"/><w:basedOn w:val="Normal"/></w:style>'
        '<w:style w:type="character" w:styleId="FootnoteReference">'
        '<w:name w:val="footnote reference"/>'
        '<w:rPr><w:vertAlign w:val="superscript"/></w:rPr></w:style>'
        "</w:styles>\n"
    )


def numbering_xml():
    bullet = decimal = ""
    for i in range(9):
        left = (i + 1) * 720
        bullet += (
            f'<w:lvl w:ilvl="{i}"><w:start w:val="1"/><w:numFmt w:val="bullet"/>'
            f'<w:lvlText w:val="•"/><w:lvlJc w:val="left"/>'
            f'<w:pPr><w:ind w:left="{left}" w:hanging="360"/></w:pPr></w:lvl>'
        )
        decimal += (
            f'<w:lvl w:ilvl="{i}"><w:start w:val="1"/><w:numFmt w:val="decimal"/>'
            f'<w:lvlText w:val="%{i + 1}."/><w:lvlJc w:val="left"/>'
            f'<w:pPr><w:ind w:left="{left}" w:hanging="360"/></w:pPr></w:lvl>'
        )
    return (
        XML_DECL
        + f'<w:numbering xmlns:w="{W}">'
        f'<w:abstractNum w:abstractNumId="0"><w:multiLevelType w:val="hybridMultilevel"/>{bullet}</w:abstractNum>'
        f'<w:abstractNum w:abstractNumId="1"><w:multiLevelType w:val="multilevel"/>{decimal}</w:abstractNum>'
        '<w:num w:numId="1"><w:abstractNumId w:val="0"/></w:num>'
        '<w:num w:numId="2"><w:abstractNumId w:val="1"/></w:num>'
        "</w:numbering>\n"
    )


NUMBERING_OVERRIDE = (
    '<Override PartName="/word/numbering.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml"/>'
)
FOOTNOTES_OVERRIDE = (
    '<Override PartName="/word/footnotes.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.footnotes+xml"/>'
)


def document_rels(entries):
    """entries: lista de (id, tipo-relativo, destino, externo?)."""
    out = (
        XML_DECL
        + '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
    )
    for rid, kind, target, external in entries:
        mode = ' TargetMode="External"' if external else ""
        out += f'<Relationship Id="{rid}" Type="{R}/{kind}" Target="{target}"{mode}/>'
    return out + "</Relationships>\n"


def png_bytes(width, height, rgb):
    """PNG mínimo válido, sin dependencias (ni PIL): un rectángulo liso."""
    raw = b"".join(b"\x00" + bytes(rgb) * width for _ in range(height))

    def chunk(tag, data):
        body = tag + data
        return struct.pack(">I", len(data)) + body + struct.pack(">I", zlib.crc32(body))

    return (
        b"\x89PNG\r\n\x1a\n"
        + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0))
        + chunk(b"IDAT", zlib.compress(raw, 9))
        + chunk(b"IEND", b"")
    )


def write_docx(name, parts):
    """Escribe el paquete. Orden y compresión no importan en OOXML (a diferencia
    del EPUB, que exige `mimetype` primero y sin comprimir)."""
    path = os.path.join(OUT_DIR, name)
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        for part, data in parts.items():
            z.writestr(part, data.encode("utf-8") if isinstance(data, str) else data)
    print(f"  {name}  ({os.path.getsize(path)} bytes)")


# --- word-tipico.docx --------------------------------------------------------

def drawing(rid, cx, cy, name):
    return (
        '<w:r><w:drawing><wp:inline distT="0" distB="0" distL="0" distR="0">'
        f'<wp:extent cx="{cx}" cy="{cy}"/><wp:docPr id="1" name="Imagen 1"/>'
        f'<a:graphic><a:graphicData uri="{PIC}">'
        f'<pic:pic><pic:nvPicPr><pic:cNvPr id="1" name="{name}"/><pic:cNvPicPr/></pic:nvPicPr>'
        f'<pic:blipFill><a:blip r:embed="{rid}"/><a:stretch><a:fillRect/></a:stretch></pic:blipFill>'
        f'<pic:spPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="{cx}" cy="{cy}"/></a:xfrm>'
        '<a:prstGeom prst="rect"><a:avLst/></a:prstGeom></pic:spPr></pic:pic>'
        "</a:graphicData></a:graphic></wp:inline></w:drawing></w:r>"
    )


def make_word_tipico():
    def li(text, num_id, ilvl=0):
        return (
            '<w:p><w:pPr><w:pStyle w:val="ListParagraph"/><w:numPr>'
            f'<w:ilvl w:val="{ilvl}"/><w:numId w:val="{num_id}"/></w:numPr></w:pPr>'
            f'<w:r><w:t xml:space="preserve">{text}</w:t></w:r></w:p>'
        )

    def code_line(text):
        return (
            '<w:p><w:pPr><w:pStyle w:val="SourceCode"/></w:pPr>'
            f'<w:r><w:t xml:space="preserve">{text}</w:t></w:r></w:p>'
        )

    def cell(text, header=False):
        rpr = "<w:rPr><w:b/></w:rPr>" if header else ""
        return (
            '<w:tc><w:tcPr><w:tcW w:w="2500" w:type="dxa"/></w:tcPr>'
            f'<w:p><w:r>{rpr}<w:t xml:space="preserve">{text}</w:t></w:r></w:p></w:tc>'
        )

    body = (
        '<w:p><w:pPr><w:pStyle w:val="Title"/></w:pPr>'
        "<w:r><w:t>Informe de pruebas</w:t></w:r></w:p>"
        '<w:p><w:pPr><w:pStyle w:val="Heading1"/></w:pPr>'
        "<w:r><w:t>Primera sección</w:t></w:r></w:p>"
        # Word parte los runs a mitad de palabra (revisión ortográfica, rsid…):
        # el importador debe recomponer «Negociación» en una sola palabra.
        '<w:p><w:r><w:t xml:space="preserve">La palabra </w:t></w:r>'
        "<w:r><w:t>Nego</w:t></w:r><w:r><w:t>ciaci</w:t></w:r>"
        '<w:r><w:t>ón</w:t></w:r><w:r><w:t xml:space="preserve"> va partida en cuatro runs.</w:t></w:r></w:p>'
        "<w:p>"
        "<w:r><w:rPr><w:b/></w:rPr><w:t>negrita</w:t></w:r>"
        '<w:r><w:t xml:space="preserve"> </w:t></w:r>'
        "<w:r><w:rPr><w:i/></w:rPr><w:t>cursiva</w:t></w:r>"
        '<w:r><w:t xml:space="preserve"> </w:t></w:r>'
        '<w:r><w:rPr><w:strike/></w:rPr><w:t>tachado</w:t></w:r>'
        '<w:r><w:t xml:space="preserve"> H</w:t></w:r>'
        '<w:r><w:rPr><w:vertAlign w:val="subscript"/></w:rPr><w:t>2</w:t></w:r>'
        '<w:r><w:t xml:space="preserve">O y x</w:t></w:r>'
        '<w:r><w:rPr><w:vertAlign w:val="superscript"/></w:rPr><w:t>2</w:t></w:r>'
        '<w:r><w:t xml:space="preserve"> y </w:t></w:r>'
        '<w:r><w:rPr><w:rStyle w:val="VerbatimChar"/></w:rPr><w:t>codigo()</w:t></w:r>'
        "</w:p>"
        # Hipervínculo como lo escribe Word: por RELACIÓN (r:id), no por campo.
        '<w:p><w:r><w:t xml:space="preserve">Visita </w:t></w:r>'
        '<w:hyperlink r:id="rId6">'
        '<w:r><w:rPr><w:rStyle w:val="Hyperlink"/></w:rPr><w:t>el sitio</w:t></w:r>'
        '</w:hyperlink><w:r><w:t xml:space="preserve"> para saber más.</w:t></w:r></w:p>'
        '<w:p><w:pPr><w:pStyle w:val="Heading2"/></w:pPr>'
        "<w:r><w:t>Listas</w:t></w:r></w:p>"
        + li("Primer punto", 1)
        + li("Segundo punto", 1)
        + li("Anidado bajo el segundo", 1, ilvl=1)
        + li("Paso uno", 2)
        + li("Paso dos", 2)
        + '<w:p><w:pPr><w:pStyle w:val="Quote"/></w:pPr>'
        "<w:r><w:t>Una cita en bloque de Word.</w:t></w:r></w:p>"
        '<w:p><w:pPr><w:pStyle w:val="Heading2"/></w:pPr>'
        "<w:r><w:t>Código y tabla</w:t></w:r></w:p>"
        + code_line("int main() {")
        + code_line("    return 0;")
        + code_line("}")
        + '<w:tbl><w:tblPr><w:tblW w:w="0" w:type="auto"/>'
        '<w:tblBorders><w:top w:val="single" w:sz="4" w:space="0" w:color="auto"/>'
        '<w:left w:val="single" w:sz="4" w:space="0" w:color="auto"/>'
        '<w:bottom w:val="single" w:sz="4" w:space="0" w:color="auto"/>'
        '<w:right w:val="single" w:sz="4" w:space="0" w:color="auto"/>'
        '<w:insideH w:val="single" w:sz="4" w:space="0" w:color="auto"/>'
        '<w:insideV w:val="single" w:sz="4" w:space="0" w:color="auto"/>'
        "</w:tblBorders></w:tblPr>"
        '<w:tblGrid><w:gridCol w:w="2500"/><w:gridCol w:w="2500"/></w:tblGrid>'
        '<w:tr><w:trPr><w:tblHeader/></w:trPr>'
        + cell("Concepto", header=True)
        + cell("Importe", header=True)
        + "</w:tr><w:tr>"
        + cell("Licencias")
        + cell("1.200 €")
        + "</w:tr><w:tr>"
        + cell("Soporte")
        + cell("300 €")
        + "</w:tr></w:tbl><w:p/>"
        '<w:p><w:pPr><w:pStyle w:val="Heading2"/></w:pPr>'
        "<w:r><w:t>Imagen</w:t></w:r></w:p>"
        "<w:p>" + drawing("rId5", 762000, 762000, "cuadro.png") + "</w:p>"
        + SECT_PR
    )

    document = (
        XML_DECL
        + f"<w:document{DOC_NS}><w:body>{body}</w:body></w:document>\n"
    )
    rels = document_rels(
        [
            ("rId1", "styles", "styles.xml", False),
            ("rId2", "numbering", "numbering.xml", False),
            ("rId5", "image", "media/image1.png", False),
            ("rId6", "hyperlink", "https://example.com/sitio", True),
        ]
    )
    write_docx(
        "word-tipico.docx",
        {
            "[Content_Types].xml": content_types([NUMBERING_OVERRIDE], png=True),
            "_rels/.rels": ROOT_RELS,
            "docProps/core.xml": core_xml("Informe de pruebas"),
            "word/document.xml": document,
            "word/_rels/document.xml.rels": rels,
            "word/styles.xml": styles_xml(),
            "word/numbering.xml": numbering_xml(),
            "word/media/image1.png": png_bytes(24, 24, (0x2E, 0x74, 0xB5)),
        },
    )


# --- word-raro.docx ----------------------------------------------------------

def make_word_raro():
    date = 'w:author="Revisor" w:date="2026-01-15T10:00:00Z"'

    nested = (
        '<w:tbl><w:tblPr><w:tblW w:w="0" w:type="auto"/></w:tblPr>'
        '<w:tblGrid><w:gridCol w:w="4000"/></w:tblGrid>'
        "<w:tr><w:tc><w:tcPr><w:tcW w:w=\"4000\" w:type=\"dxa\"/></w:tcPr>"
        "<w:p><w:r><w:t>Celda externa</w:t></w:r></w:p>"
        # Tabla ANIDADA dentro de la celda; tras ella hace falta un <w:p/>.
        '<w:tbl><w:tblPr><w:tblW w:w="0" w:type="auto"/></w:tblPr>'
        '<w:tblGrid><w:gridCol w:w="1800"/></w:tblGrid>'
        '<w:tr><w:tc><w:tcPr><w:tcW w:w="1800" w:type="dxa"/></w:tcPr>'
        "<w:p><w:r><w:t>Celda anidada</w:t></w:r></w:p></w:tc></w:tr></w:tbl>"
        "<w:p/></w:tc></w:tr></w:tbl><w:p/>"
    )

    body = (
        '<w:p><w:pPr><w:pStyle w:val="Heading1"/></w:pPr>'
        '<w:bookmarkStart w:id="0" w:name="_Toc0001"/>'
        "<w:r><w:t>Casos raros</w:t></w:r>"
        '<w:bookmarkEnd w:id="0"/></w:p>'
        # Metacaracteres XML, escapados en el fichero: al importar deben volver
        # a ser los caracteres literales.
        '<w:p><w:proofErr w:type="spellStart"/>'
        "<w:r><w:t>Signos: &amp; &lt; &gt; &quot; &apos; y un asterisco * literal.</w:t></w:r>"
        '<w:proofErr w:type="spellEnd"/></w:p>'
        # Fuera del BMP (par suplente en UTF-16) + acento combinante.
        "<w:p><w:r><w:t>Emoji \U0001F9EA y combinante é.</w:t></w:r></w:p>"
        # Cambios controlados: lo insertado se queda, lo borrado NO debe aparecer.
        f'<w:p><w:ins w:id="1" {date}>'
        '<w:r><w:t xml:space="preserve">Texto INSERTADO </w:t></w:r></w:ins>'
        f'<w:del w:id="2" {date}>'
        "<w:r><w:delText>ESTOSEBORRO</w:delText></w:r></w:del>"
        "<w:r><w:t>y el resto.</w:t></w:r></w:p>"
        # Tabulador y salto de línea suave dentro de un mismo run.
        '<w:p><w:r><w:t>antes</w:t><w:tab/><w:t>después</w:t><w:br/>'
        "<w:t>segunda línea del mismo párrafo</w:t></w:r></w:p>"
        # Párrafo vacío y párrafo con solo ruido de Word.
        "<w:p/>"
        "<w:p><w:r><w:lastRenderedPageBreak/>"
        "<w:t>Tras un salto de página renderizado.</w:t></w:r></w:p>"
        # Nota al pie (parte word/footnotes.xml).
        '<w:p><w:r><w:t>Frase con nota</w:t></w:r>'
        '<w:r><w:rPr><w:rStyle w:val="FootnoteReference"/></w:rPr>'
        '<w:footnoteReference w:id="2"/></w:r>'
        "<w:r><w:t>.</w:t></w:r></w:p>"
        + nested
        + SECT_PR
    )

    footnotes = (
        XML_DECL
        + f'<w:footnotes xmlns:w="{W}">'
        '<w:footnote w:type="separator" w:id="0"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>'
        '<w:footnote w:type="continuationSeparator" w:id="1">'
        "<w:p><w:r><w:continuationSeparator/></w:r></w:p></w:footnote>"
        '<w:footnote w:id="2"><w:p><w:pPr><w:pStyle w:val="FootnoteText"/></w:pPr>'
        '<w:r><w:rPr><w:rStyle w:val="FootnoteReference"/></w:rPr><w:footnoteRef/></w:r>'
        '<w:r><w:t xml:space="preserve"> El texto de la nota al pie.</w:t></w:r>'
        "</w:p></w:footnote></w:footnotes>\n"
    )

    document = (
        XML_DECL
        + f"<w:document{DOC_NS}><w:body>{body}</w:body></w:document>\n"
    )
    rels = document_rels(
        [
            ("rId1", "styles", "styles.xml", False),
            ("rId2", "numbering", "numbering.xml", False),
            ("rId3", "footnotes", "footnotes.xml", False),
        ]
    )
    write_docx(
        "word-raro.docx",
        {
            "[Content_Types].xml": content_types(
                [NUMBERING_OVERRIDE, FOOTNOTES_OVERRIDE]
            ),
            "_rels/.rels": ROOT_RELS,
            "docProps/core.xml": core_xml("Casos raros"),
            "word/document.xml": document,
            "word/_rels/document.xml.rels": rels,
            "word/styles.xml": styles_xml(),
            "word/numbering.xml": numbering_xml(),
            "word/footnotes.xml": footnotes,
        },
    )


# --- vacio.docx --------------------------------------------------------------

def make_vacio():
    document = (
        XML_DECL
        + f"<w:document{DOC_NS}><w:body><w:p/>{SECT_PR}</w:body></w:document>\n"
    )
    write_docx(
        "vacio.docx",
        {
            "[Content_Types].xml": content_types(),
            "_rels/.rels": ROOT_RELS,
            "docProps/core.xml": core_xml("Sin contenido"),
            "word/document.xml": document,
            "word/_rels/document.xml.rels": document_rels(
                [("rId1", "styles", "styles.xml", False)]
            ),
            "word/styles.xml": styles_xml(),
        },
    )


# --- pandoc-basico.docx ------------------------------------------------------

MARKDOWN_BASICO = """# Documento de otra herramienta

Un párrafo con **negrita**, *cursiva* y `código en línea`.

## Listas

- primer elemento
- segundo elemento
  - anidado

1. paso uno
2. paso dos

## Cita y código

> Una cita en bloque.

```python
def saluda():
    return "hola"
```

## Tabla

| Concepto | Importe |
|:---------|--------:|
| Licencias | 1.200 € |
| Soporte | 300 € |

Un [enlace](https://example.com/destino) y una regla:

---

Fin.
"""


def make_pandoc_basico():
    src = os.path.join(OUT_DIR, "pandoc-basico.md")
    out = os.path.join(OUT_DIR, "pandoc-basico.docx")
    with open(src, "w", encoding="utf-8") as f:
        f.write(MARKDOWN_BASICO)
    try:
        subprocess.run(
            ["pandoc", "--from=gfm", "--to=docx", "--output", out, src],
            check=True,
        )
    except (OSError, subprocess.CalledProcessError) as e:
        print(f"  AVISO: no se pudo generar pandoc-basico.docx ({e}); se conserva el existente")
        return
    print(f"  pandoc-basico.docx  ({os.path.getsize(out)} bytes)")


def main():
    os.makedirs(OUT_DIR, exist_ok=True)
    print(f"Generando fixtures en {OUT_DIR}:")
    make_pandoc_basico()
    make_word_tipico()
    make_word_raro()
    make_vacio()
    return 0


if __name__ == "__main__":
    sys.exit(main())
