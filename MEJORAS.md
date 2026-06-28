# Posibles mejoras

Lista de mejoras pendientes, ordenadas por relación valor/coste y riesgo
(de menor a mayor). Se implementan una por una, cada una con su test y commit.

- [x] **Interlineado configurable en pantalla.** Hoy el editor usa el interlineado
  por defecto de Qt (factor 1.0) y no es ajustable. Añadir un ajuste en *Ver*
  (p. ej. Sencillo / Medio / Amplio) que aplique `QTextBlockFormat::setLineHeight`
  a los bloques al cargar/formatear. No afecta al round-trip (no se serializa).
  Coste bajo.

- [x] **El modo sin distracciones no debería salirse al cambiar de pestaña.**
  Limitación documentada: F11 se desactiva al cambiar de documento. Como el
  controlador ya es de la ventana (`DistractionFreeController::setTargets`),
  reengancharlo a la nueva pestaña en vez de salir. Coste bajo-medio.

- [x] **Resaltado de sintaxis en los bloques de código al exportar.** En pantalla
  se resaltan; revisar si HTML/PDF/DOCX/ODF lo conservan y, si no, emitir el
  código con color. Coste medio.

- [ ] **Recuperación por pestaña.** El borrador de autoguardado es de ruta fija
  (`recovery-draft.md`, compartido por todas las pestañas: recupera solo el último
  editado). Con varias pestañas y un cierre inesperado se pierde el resto.
  Indexar los borradores por pestaña/ruta y ofrecer recuperar todos al arrancar.
  Coste medio. Es la única laguna real de seguridad de datos.
