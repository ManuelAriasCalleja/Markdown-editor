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
            "- [ ] [tarea]\n")});

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
            "## Notas\n")});

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
            "## Conclusión\n")});

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
            "[Licencia]\n")});

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
            "[Nombre]\n")});

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
            "## 4. Conclusiones\n")});

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
            "- [x] [tarea]\n")});

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
            "Fdo.: [Nombre y cargo]\n")});

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
            "## Bibliografía\n")});

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
            "**3.** [Enunciado] *([N] puntos)*\n")});

    return list;
}

}  // namespace mdtemplate
