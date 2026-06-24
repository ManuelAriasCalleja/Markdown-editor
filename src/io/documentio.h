#ifndef DOCUMENTIO_H
#define DOCUMENTIO_H

/// \file
/// \brief Lectura/escritura del documento Markdown sobre un QTextEdit.

#include <QObject>
#include <QString>

class QTextEdit;

/// \brief Lee y escribe el documento Markdown sobre un QTextEdit: gestiona la
/// codificación (UTF-8), la baseUrl para los recursos relativos, la
/// serialización (setMarkdown/toMarkdown) y la ruta del archivo actual.
///
/// No muestra diálogos ni mensajes: informa de los errores por el valor de
/// retorno y de los cambios de estado por señales, para no acoplarse a la vista.
class DocumentIo : public QObject
{
    Q_OBJECT

public:
    explicit DocumentIo(QTextEdit *editor, QObject *parent = nullptr);

    /// \brief Carga `path` en el editor. Devuelve false y rellena *errorString si falla.
    bool load(const QString &path, QString *errorString);
    /// \brief Escribe el documento en `path`. Devuelve false y rellena *errorString.
    bool write(const QString &path, QString *errorString);

    /// \brief Ruta del archivo asociado al documento actual ("" si no hay).
    QString currentFile() const;
    /// \brief Modificado = el contenido (Markdown) difiere del último guardado/cargado.
    /// Se compara con una línea base en vez de fiarse de QTextDocument::isModified,
    /// que QTextEdit ensucia espuriamente al trazar por primera vez.
    bool isModified() const;

    /// \brief Empieza un documento nuevo y vacío (sin archivo asociado).
    void reset();

    /// \brief Carga `content` (Markdown) como documento nuevo sin archivo asociado: para
    /// «Nuevo desde plantilla». Separa el front matter como en load(), pero deja el
    /// documento como modificado y sin ruta (Guardar pedirá nombre), de modo que la
    /// plantilla no se pierda sin avisar.
    void loadFromString(const QString &content);

    /// \brief ¿El documento cargado tenía front matter (--- … --- o +++ … +++) al
    /// principio? Se conserva verbatim, sin renderizarlo, y se reescribe al
    /// guardar.
    bool hasFrontMatter() const;
    /// \brief El bloque de front matter verbatim (delimitadores incluidos), o "". Útil
    /// para leer metadatos como el idioma del documento al exportar.
    QString frontMatter() const;

signals:
    /// \brief El archivo asociado cambió (al cargar, guardar o empezar uno nuevo).
    void currentFileChanged(const QString &path);
    /// \brief Se acaba de cargar contenido (p. ej. para que la vista recoloree enlaces).
    void documentLoaded();

private:
    void setCurrentFile(const QString &path);

    QTextEdit *m_editor;
    QString m_currentFile;
    QString m_baseline;     // Markdown en el último guardado/carga (para isModified)
    QString m_frontMatter;  // bloque de front matter a conservar (vacío = sin él)
};

#endif // DOCUMENTIO_H
