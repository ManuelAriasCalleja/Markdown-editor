/// \file
/// \brief Definición de las plantillas de documento (esqueletos Markdown traducidos).

#include "doctemplates.h"

#include <QCoreApplication>

namespace mdtemplate {

QList<DocTemplate> all()
{
    QList<DocTemplate> list;

    // Acta de reunión
    list.append({
        QCoreApplication::translate("MainWindow", "Acta de reunión"),
        QCoreApplication::translate("MainWindow",
            "---\n"
            "title: Acta de reunión\n"
            "date: [fecha]\n"
            "---\n"
            "\n"
            "# Acta de reunión\n"
            "\n"
            "**Asistentes:** [nombres]\n"
            "\n"
            "## Orden del día\n"
            "\n"
            "1. [punto]\n"
            "\n"
            "## Acuerdos\n"
            "\n"
            "- [acuerdo]\n"
            "\n"
            "## Tareas\n"
            "\n"
            "- [ ] [tarea]\n"),
        Category::Business});

    // Nota diaria
    list.append({
        QCoreApplication::translate("MainWindow", "Nota diaria"),
        QCoreApplication::translate("MainWindow",
            "# [fecha]\n"
            "\n"
            "## Hecho hoy\n"
            "\n"
            "- \n"
            "\n"
            "## Pendiente\n"
            "\n"
            "- [ ] \n"
            "\n"
            "## Notas\n"),
        Category::Personal});

    // Artículo de blog
    list.append({
        QCoreApplication::translate("MainWindow", "Artículo de blog"),
        QCoreApplication::translate("MainWindow",
            "---\n"
            "title: [Título]\n"
            "date: [fecha]\n"
            "tags: []\n"
            "draft: true\n"
            "---\n"
            "\n"
            "# [Título]\n"
            "\n"
            "## Introducción\n"
            "\n"
            "## Desarrollo\n"
            "\n"
            "## Conclusión\n"),
        Category::Writing});

    // README de proyecto
    list.append({
        QCoreApplication::translate("MainWindow", "README de proyecto"),
        QCoreApplication::translate("MainWindow",
            "# [Nombre del proyecto]\n"
            "\n"
            "[Descripción breve del proyecto.]\n"
            "\n"
            "## Instalación\n"
            "\n"
            "```\n"
            "[comandos de instalación]\n"
            "```\n"
            "\n"
            "## Uso\n"
            "\n"
            "## Licencia\n"
            "\n"
            "[Licencia]\n"),
        Category::Programming});

    // Carta
    list.append({
        QCoreApplication::translate("MainWindow", "Carta"),
        QCoreApplication::translate("MainWindow",
            "[Remitente]\n"
            "[Dirección]\n"
            "\n"
            "[Lugar], [fecha]\n"
            "\n"
            "[Destinatario]\n"
            "\n"
            "Estimado/a [nombre]:\n"
            "\n"
            "[Cuerpo de la carta.]\n"
            "\n"
            "Atentamente,\n"
            "\n"
            "[Nombre]\n"),
        Category::Personal});

    // Informe
    list.append({
        QCoreApplication::translate("MainWindow", "Informe"),
        QCoreApplication::translate("MainWindow",
            "---\n"
            "title: [Título del informe]\n"
            "author: [Autor]\n"
            "date: [fecha]\n"
            "---\n"
            "\n"
            "# [Título del informe]\n"
            "\n"
            "## Resumen\n"
            "\n"
            "## 1. Introducción\n"
            "\n"
            "## 2. Desarrollo\n"
            "\n"
            "## 3. Resultados\n"
            "\n"
            "## 4. Conclusiones\n"),
        Category::Business});

    // Lista de tareas
    list.append({
        QCoreApplication::translate("MainWindow", "Lista de tareas"),
        QCoreApplication::translate("MainWindow",
            "# [Título]\n"
            "\n"
            "## Pendiente\n"
            "\n"
            "- [ ] [tarea]\n"
            "- [ ] [tarea]\n"
            "\n"
            "## Hecho\n"
            "\n"
            "- [x] [tarea]\n"),
        Category::Personal});

    // Certificado (la palabra CERTIFICO va como encabezado para que se vea grande
    // y sobreviva al guardado: Markdown no expresa tamaño de fuente, solo niveles
    // de encabezado).
    list.append({
        QCoreApplication::translate("MainWindow", "Certificado"),
        QCoreApplication::translate("MainWindow",
            "Yo, **[Nombre y apellidos]**, con DNI/NIF **[número]**, en calidad de "
            "**[cargo]** de **[organización]**,\n"
            "\n"
            "# CERTIFICO:\n"
            "\n"
            "Que **[nombre]** [hecho que se certifica].\n"
            "\n"
            "Y para que así conste a los efectos oportunos, firmo el presente "
            "certificado.\n"
            "\n"
            "En [lugar], a [fecha].\n"
            "\n"
            "Fdo.: [Nombre y cargo]\n"),
        Category::Personal});

    // Práctica de asignatura
    list.append({
        QCoreApplication::translate("MainWindow", "Práctica de asignatura"),
        QCoreApplication::translate("MainWindow",
            "---\n"
            "title: Práctica [N] — [Título]\n"
            "author: [Nombre del alumno]\n"
            "date: [fecha]\n"
            "---\n"
            "\n"
            "# Práctica [N] — [Título]\n"
            "\n"
            "**Asignatura:** [Asignatura] · **Autor:** [Nombre] · **Grupo:** [Grupo]\n"
            "\n"
            "## 1. Objetivos\n"
            "\n"
            "## 2. Material y métodos\n"
            "\n"
            "## 3. Desarrollo\n"
            "\n"
            "## 4. Resultados\n"
            "\n"
            "## 5. Conclusiones\n"
            "\n"
            "## Bibliografía\n"),
        Category::Teaching});

    // Examen
    list.append({
        QCoreApplication::translate("MainWindow", "Examen"),
        QCoreApplication::translate("MainWindow",
            "---\n"
            "title: Examen — [Asignatura]\n"
            "date: [fecha]\n"
            "---\n"
            "\n"
            "# [Asignatura] — Examen\n"
            "\n"
            "**Fecha:** [fecha] · **Duración:** [tiempo] · **Nombre:** "
            "____________________\n"
            "\n"
            "> Instrucciones: [puntuación, material permitido, etc.]\n"
            "\n"
            "**1.** [Enunciado] *([N] puntos)*\n"
            "\n"
            "**2.** [Enunciado] *([N] puntos)*\n"
            "\n"
            "**3.** [Enunciado] *([N] puntos)*\n"),
        Category::Teaching});

    // Registro de cambios (CHANGELOG)
    list.append({
        QCoreApplication::translate("MainWindow", "Registro de cambios (CHANGELOG)"),
        QCoreApplication::translate("MainWindow",
            "# Registro de cambios\n"
            "\n"
            "Todos los cambios notables de este proyecto se documentan en este archivo.\n"
            "\n"
            "## [Sin publicar]\n"
            "\n"
            "### Añadido\n"
            "\n"
            "- [nueva funcionalidad]\n"
            "\n"
            "### Cambiado\n"
            "\n"
            "- [cambio en algo existente]\n"
            "\n"
            "### Corregido\n"
            "\n"
            "- [error corregido]\n"),
        Category::Programming});

    // Registro de decisión de arquitectura (ADR)
    list.append({
        QCoreApplication::translate("MainWindow", "Decisión de arquitectura (ADR)"),
        QCoreApplication::translate("MainWindow",
            "# ADR [N]: [Título de la decisión]\n"
            "\n"
            "- **Estado:** propuesto\n"
            "- **Fecha:** [fecha]\n"
            "\n"
            "## Contexto\n"
            "\n"
            "[Qué problema o necesidad motiva esta decisión.]\n"
            "\n"
            "## Decisión\n"
            "\n"
            "[La decisión tomada, en voz activa: «Usaremos…».]\n"
            "\n"
            "## Consecuencias\n"
            "\n"
            "[Qué se vuelve más fácil o más difícil a raíz de esta decisión.]\n"
            "\n"
            "## Alternativas consideradas\n"
            "\n"
            "- [alternativa] — [por qué se descartó]\n"),
        Category::Programming});

    // Informe de error (bug)
    list.append({
        QCoreApplication::translate("MainWindow", "Informe de error"),
        QCoreApplication::translate("MainWindow",
            "# [Título breve del error]\n"
            "\n"
            "## Descripción\n"
            "\n"
            "[Qué ocurre.]\n"
            "\n"
            "## Pasos para reproducir\n"
            "\n"
            "1. [paso]\n"
            "2. [paso]\n"
            "3. [paso]\n"
            "\n"
            "## Resultado esperado\n"
            "\n"
            "[Lo que debería ocurrir.]\n"
            "\n"
            "## Resultado obtenido\n"
            "\n"
            "[Lo que ocurre en realidad.]\n"
            "\n"
            "## Entorno\n"
            "\n"
            "- **Versión:** [versión]\n"
            "- **Sistema:** [sistema operativo]\n"),
        Category::Programming});

    // Artículo científico (IMRyD)
    list.append({
        QCoreApplication::translate("MainWindow", "Artículo científico (IMRyD)"),
        QCoreApplication::translate("MainWindow",
            "---\n"
            "title: [Título del artículo]\n"
            "author: [Autores]\n"
            "date: [fecha]\n"
            "---\n"
            "\n"
            "# [Título del artículo]\n"
            "\n"
            "## Resumen\n"
            "\n"
            "[Resumen breve del trabajo.]\n"
            "\n"
            "**Palabras clave:** [palabra1, palabra2, palabra3]\n"
            "\n"
            "## 1. Introducción\n"
            "\n"
            "[Contexto, problema y objetivo del estudio.]\n"
            "\n"
            "## 2. Métodos\n"
            "\n"
            "[Diseño, materiales y procedimiento, de forma reproducible.]\n"
            "\n"
            "## 3. Resultados\n"
            "\n"
            "[Hallazgos, con tablas o figuras si procede.]\n"
            "\n"
            "## 4. Discusión\n"
            "\n"
            "[Interpretación, limitaciones y comparación con trabajos previos.]\n"
            "\n"
            "## Referencias\n"
            "\n"
            "1. [Referencia]\n"),
        Category::Academic});

    // Informe de laboratorio
    list.append({
        QCoreApplication::translate("MainWindow", "Informe de laboratorio"),
        QCoreApplication::translate("MainWindow",
            "---\n"
            "title: [Título de la práctica]\n"
            "author: [Nombre]\n"
            "date: [fecha]\n"
            "---\n"
            "\n"
            "# [Título de la práctica]\n"
            "\n"
            "## Objetivo\n"
            "\n"
            "[Qué se pretende demostrar o medir.]\n"
            "\n"
            "## Fundamento teórico\n"
            "\n"
            "[Base teórica y fórmulas relevantes.]\n"
            "\n"
            "## Materiales\n"
            "\n"
            "- [material]\n"
            "\n"
            "## Procedimiento\n"
            "\n"
            "1. [paso]\n"
            "\n"
            "## Resultados\n"
            "\n"
            "[Datos y observaciones; tablas de medidas.]\n"
            "\n"
            "## Análisis y conclusiones\n"
            "\n"
            "[Cálculo de errores, interpretación y conclusión.]\n"),
        Category::Academic});

    return list;
}

QString categoryName(Category category)
{
    switch (category) {
    case Category::Personal:
        return QCoreApplication::translate("MainWindow", "Personal y general");
    case Category::Programming:
        return QCoreApplication::translate("MainWindow", "Programación");
    case Category::Academic:
        return QCoreApplication::translate("MainWindow", "Académico");
    case Category::Teaching:
        return QCoreApplication::translate("MainWindow", "Docencia");
    case Category::Business:
        return QCoreApplication::translate("MainWindow", "Empresa");
    case Category::Legal:
        return QCoreApplication::translate("MainWindow", "Derecho");
    case Category::Writing:
        return QCoreApplication::translate("MainWindow", "Escritura");
    }
    return {};
}

QList<Category> categoriesInOrder()
{
    return {Category::Personal, Category::Programming, Category::Academic,
            Category::Teaching, Category::Business,    Category::Legal,
            Category::Writing};
}

QString welcomeDocument()
{
    return QCoreApplication::translate("MainWindow",
        "# ¡Te damos la bienvenida a md-editor!\n"
        "\n"
        "Este es un editor **visual** de Markdown: escribes y das formato sobre el "
        "texto ya renderizado, sin ver el código.\n"
        "\n"
        "## Para empezar\n"
        "\n"
        "- Da formato con la **barra de herramientas** o tecleando Markdown: `## ` para "
        "un encabezado, `- ` para una lista, `**negrita**`…\n"
        "- Pulsa **F1** para abrir el manual completo.\n"
        "- Crea un documento nuevo con **Ctrl+N** o abre uno con **Ctrl+O**.\n"
        "\n"
        "Borra este texto y empieza a escribir.\n");
}

}  // namespace mdtemplate
