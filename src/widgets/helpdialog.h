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

protected:
    // Recalcula el ancho del índice cuando cambia la fuente (el zoom de la
    // interfaz se la fija desde MainWindow tras crear el diálogo).
    void changeEvent(QEvent *event) override;

private:
    // Carga un .md del recurso /help y lo muestra en el visor.
    void loadPage(const QString &resourcePath);
    // Ajusta el ancho de la columna del índice al de sus rótulos con la fuente
    // actual. No puede ser un ancho fijo en píxeles: con el zoom subido (o en un
    // idioma de rótulos largos) el texto se veía recortado.
    void updateIndexWidth();
    // Da a la ventana el tamaño de lectura que corresponde a la fuente actual
    // (proporcional a cuánto ha crecido respecto a la de la aplicación, y como mucho
    // lo que quepa en la pantalla): con la fuente ampliada, en el mismo ancho cabe
    // menos texto y las líneas se parten de forma poco natural.
    void updateWindowSize();

    QListWidget *m_index;
    QTextBrowser *m_viewer;
};

#endif // HELPDIALOG_H
