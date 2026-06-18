#include "documentio.h"

#include "footnotes.h"
#include "mathblocks.h"
#include "tableedit.h"

#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStringConverter>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextStream>
#include <QUrl>

namespace {

// Si `content` empieza por un bloque de front matter en su propia línea
// (--- … --- de YAML o +++ … +++ de TOML), lo separa: devuelve el bloque
// (delimitadores incluidos) y deja en `content` solo el cuerpo. Si no hay front
// matter, devuelve "" y no toca `content`. Así setMarkdown() no lo malinterpreta
// como una regla horizontal y los metadatos se conservan tal cual.
QString takeFrontMatter(QString &content)
{
    static const QRegularExpression re(
        QStringLiteral("\\A(---|\\+\\+\\+)[ \\t]*\\r?\\n.*?\\r?\\n\\1[ \\t]*\\r?(\\n|\\z)"),
        QRegularExpression::DotMatchesEverythingOption);
    const QRegularExpressionMatch m = re.match(content);
    if (!m.hasMatch())
        return QString();
    const QString frontMatter = m.captured(0);
    content.remove(0, frontMatter.length());
    return frontMatter;
}

} // namespace

DocumentIo::DocumentIo(QTextEdit *editor, QObject *parent)
    : QObject(parent), m_editor(editor)
{
}

QString DocumentIo::currentFile() const
{
    return m_currentFile;
}

bool DocumentIo::isModified() const
{
    return mdtable::documentMarkdown(m_editor->document()) != m_baseline;
}

bool DocumentIo::hasFrontMatter() const
{
    return !m_frontMatter.isEmpty();
}

QString DocumentIo::frontMatter() const
{
    return m_frontMatter;
}

void DocumentIo::setCurrentFile(const QString &path)
{
    m_currentFile = path;
    // Fija la línea base con el contenido ya cargado/guardado: a partir de aquí
    // "modificado" significa que el usuario lo cambió respecto a esto.
    m_baseline = mdtable::documentMarkdown(m_editor->document());
    m_editor->document()->setModified(false);
    emit currentFileChanged(path);
}

void DocumentIo::reset()
{
    m_editor->clear();
    // clear() no reinicia el formato de inserción del cursor: lo restablecemos
    // para que el documento nuevo empiece sin formato heredado.
    m_editor->setCurrentCharFormat(QTextCharFormat());
    m_frontMatter.clear();  // documento nuevo: sin front matter
    setCurrentFile(QString());
}

bool DocumentIo::load(const QString &path, QString *errorString)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorString)
            *errorString = file.errorString();
        return false;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QString content = in.readAll();
    file.close();

    // Separa el front matter (si lo hay) antes de renderizar: se conserva
    // verbatim y se reescribe al guardar, en vez de dejarlo a merced de
    // setMarkdown() (que lo tomaría por una regla horizontal y texto suelto).
    m_frontMatter = takeFrontMatter(content);

    // Resuelve imágenes y enlaces relativos respecto al directorio del archivo
    // (debe fijarse antes de setMarkdown para que se carguen al maquetar).
    const QFileInfo info(path);
    m_editor->document()->setBaseUrl(
        QUrl::fromLocalFile(info.absolutePath() + QLatin1Char('/')));

    // Protege las fórmulas $...$/$$...$$ del re-parseo de setMarkdown(), que
    // interpretaría `_`/`*` dentro como cursiva/negrita. mdtable::documentMarkdown
    // las desenvuelve al serializar.
    m_editor->setMarkdown(mdmath::protectMath(mdfootnote::protectFootnotes(content)));
    // Tras el parseo, las fórmulas están como inline-code `$tex$`. Las convertimos
    // a fragmentos «renderizados» (Unicode visible + TeX en propiedad).
    mdmath::renderMathInDocument(m_editor->document());
    // Da estilo de superíndice a las referencias de nota al pie `[^id]` (no toca
    // la serialización: el round-trip de las footnotes ya es transparente).
    mdfootnote::renderFootnotesInDocument(m_editor->document());

    setCurrentFile(path);
    emit documentLoaded();
    return true;
}

bool DocumentIo::write(const QString &path, QString *errorString)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (errorString)
            *errorString = file.errorString();
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    // Reescribe el front matter conservado delante del cuerpo, con una línea en
    // blanco de separación.
    if (!m_frontMatter.isEmpty()) {
        QString fm = m_frontMatter;
        if (!fm.endsWith(QLatin1Char('\n')))
            fm += QLatin1Char('\n');
        out << fm << '\n';
    }
    out << mdtable::documentMarkdown(m_editor->document());
    file.close();

    setCurrentFile(path);
    return true;
}
