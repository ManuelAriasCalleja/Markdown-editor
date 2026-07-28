#ifndef HELPDIALOG_H
#define HELPDIALOG_H

/// \file
/// \brief Ventana de ayuda integrada (manual de uso y guía de Markdown).

#include <QDialog>

class QListWidget;
class QTextBrowser;

/// \brief Lógica pura de la ayuda (sin GUI), para poder probarla aislada.
namespace mdhelp {

/// \brief Ancla (slug) de un encabezado, con la misma regla que usan los enlaces
/// del índice en los .md de ayuda: minúsculas, sin acentos (se descomponen y se
/// descartan las marcas), la puntuación se borra y los espacios/guiones colapsan
/// a un único guion. Así `## Enlaces e imágenes` → `enlaces-e-imagenes`, que es
/// el destino que escribe `[Enlaces e imágenes](#enlaces-e-imagenes)`.
QString headingSlug(const QString &text);

}  // namespace mdhelp

/// \brief Ventana de ayuda con dos secciones: «Uso de la aplicación» y «Markdown».
/// El contenido vive en dos .md empaquetados en el recurso /help, que se
/// renderizan con QTextBrowser::setMarkdown (el mismo motor que el editor),
/// así que la guía de Markdown se demuestra a sí misma. Es no modal: el
/// usuario puede dejarla abierta a un lado mientras escribe.
class HelpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HelpDialog(QWidget *parent = nullptr);

private:
    // Carga un .md del recurso /help y lo muestra en el visor.
    void loadPage(const QString &resourcePath);

    QListWidget *m_index;
    QTextBrowser *m_viewer;
};

#endif // HELPDIALOG_H
