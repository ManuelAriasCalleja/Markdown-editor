# Descargas — histórico de releases borradas

El 2026-07-01 se borraron las releases v1.0.0–v2.3.0 de GitHub (conservando sus
tags), dejando solo la última como release viva. GitHub **no conserva** las
descargas de releases eliminadas: el `download_count` de un asset desaparece con la
release y no hay API para consultarlo (la API de tráfico solo cubre clones/vistas de
los últimos 14 días y no incluye descargas de *release assets*).

Esta tabla es el **snapshot capturado justo antes del borrado** (2026-07-01), y es la
única copia recuperable de esos datos.

## Descargas por release (suma de los 3 binarios)

| Release | Descargas |
|---------|-----------|
| v2.3.0  | 2 |
| v2.2.0  | 2 |
| v2.1.0  | 0 |
| v2.0.0  | 9 |
| v1.3.0  | 1 |
| v1.2.0  | 3 |
| v1.1.1  | 0 |
| v1.1.0  | 4 |
| v1.0.2  | 8 |
| v1.0.1  | 0 |
| v1.0.0  | 9 |
| **Total** | **38** |

## Limitación

No hay **desglose por sistema operativo** (Linux/Windows/macOS) de estas releases: la
captura sumaba por release, no por asset, y GitHub ya no puede aportarlo. Por eso
`show-downloads.sh` incorpora este histórico solo como **total** (constante
`HISTORICAL_DELETED`), sumándolo a las descargas de las releases vivas.
