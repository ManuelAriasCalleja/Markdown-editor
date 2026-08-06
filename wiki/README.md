# Contenido de la wiki

Esta carpeta contiene las páginas de la **wiki de GitHub** de md-editor. No forma
parte del programa: son los `.md` que se copian al repositorio de la wiki.

Se generan a partir de [`../docs/REQUISITOS.md`](../docs/REQUISITOS.md), que es la
descripción completa del producto. Si actualizas la especificación, refleja aquí los
cambios relevantes.

## Páginas

| Archivo | Página en la wiki |
|---|---|
| `Home.md` | Inicio (español) |
| `Instalacion.md` | Instalación |
| `Uso.md` | Uso |
| `Caracteristicas.md` | Características |
| `Atajos.md` | Atajos de teclado |
| `*-en`, `-de`, `-fr`, `-it`, `-pt`, `-pl`, `-nl`, `-ro`, `-zh` | las mismas cinco páginas en los otros nueve idiomas (`Home-en.md`, `Uso-zh.md`…) |
| `_Sidebar.md` | Barra lateral (visible en todas las páginas) |
| `_Footer.md` | Pie con el selector de idioma (visible en todas las páginas) |

## Publicar en la wiki de GitHub

La wiki es un repositorio git aparte (`<repo>.wiki.git`).

**Paso 1 — obligatorio, solo la primera vez.** GitHub no crea ese repositorio hasta
que existe la primera página, y esa página **solo se puede crear desde el navegador**
(no por git). Si intentas clonar antes, da `Repository not found`. Ve a
`https://github.com/<usuario>/<repo>/wiki`, pulsa **Create the first page**, guarda
cualquier contenido (lo sobrescribiremos) y pulsa **Save Page**.

**Paso 2 — clonar, copiar y publicar:**

```bash
git clone https://github.com/<usuario>/<repo>.wiki.git
cd <repo>.wiki
cp /ruta/a/md-editor/wiki/*.md .          # incluye _Sidebar.md y _Footer.md
rm README.md                              # este archivo es la guía, no una página
git add .
git commit -m "Actualizar la wiki desde docs/REQUISITOS.md"
git push
```

GitHub muestra `_Sidebar.md` y `_Footer.md` en todas las páginas, así que el menú
lateral y el selector de idioma solo se escriben una vez.

## Añadir más idiomas

El proyecto tiene interfaz en 10 idiomas. Para traducir la wiki, duplica cada página
con el sufijo del idioma (`Instalacion-en`, `Uso-fr`, etc.), enlázalas en
`_Sidebar.md` y amplía el selector de `_Footer.md`. Por ejemplo:

```markdown
🌐 [ES](Home) · [EN](Home-en) · [DE](Home-de) · [FR](Home-fr) · [IT](Home-it) · [PT](Home-pt) · [PL](Home-pl) · [NL](Home-nl) · [RO](Home-ro) · [ZH](Home-zh)
```

El sufijo es de **dos letras** aunque el idioma tenga variantes (`-zh`, no `-zh_CN`):
es el esquema que ya usan las demás páginas y GitHub no lo interpreta, son solo
nombres de página. El día que se añada el chino tradicional habrá que decidir uno
(`-zh-tw`), pero no antes.

Si mantener 10 idiomas a mano se hace pesado, conviene plantear una migración a
**GitHub Pages** con un generador estático (Docusaurus o MkDocs) que ofrezca i18n y
selector de idioma automáticos.
