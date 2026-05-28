#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include <QDialog>

class QListWidget;
class QTextBrowser;

// Ventana de ayuda con dos secciones: «Uso de la aplicación» y «Markdown».
// El contenido vive en dos .md empaquetados en el recurso /help, que se
// renderizan con QTextBrowser::setMarkdown (el mismo motor que el editor),
// así que la guía de Markdown se demuestra a sí misma. Es no modal: el
// usuario puede dejarla abierta a un lado mientras escribe.
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
