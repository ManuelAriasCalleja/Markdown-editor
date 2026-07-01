#include <QtTest>

#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>

#include "documentio.h"

// Pruebas de DocumentIo: ida y vuelta de carga/guardado, errores y señales.
class TestDocumentIo : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void writeThenLoadRoundTrips();
    void loadMissingFileFails();
    void writeUpdatesCurrentFileAndClearsModified();
    void isModifiedTracksContentAgainstBaseline();
    void resetClearsAndSignals();
    void loadFromStringIsUntitledModifiedAndKeepsFrontMatter();
    void loadFromStringNotModifiedWhenAsModifiedFalse();
    void loadEmitsDocumentLoaded();
    void frontMatterIsPreserved();
    void noFrontMatterWhenNotAtStart();

private:
    QTemporaryDir m_dir;  // directorio temporal por prueba
    QString pathFor(const QString &name) const
    {
        return m_dir.path() + QLatin1Char('/') + name;
    }
    // Escribe `text` (UTF-8) en un archivo del directorio temporal.
    QString writeRaw(const QString &name, const QString &text) const
    {
        const QString path = pathFor(name);
        QFile f(path);
        f.open(QIODevice::WriteOnly | QIODevice::Text);
        f.write(text.toUtf8());
        f.close();
        return path;
    }
    static QString readRaw(const QString &path)
    {
        QFile f(path);
        f.open(QIODevice::ReadOnly | QIODevice::Text);
        const QString s = QString::fromUtf8(f.readAll());
        f.close();
        return s;
    }
};

void TestDocumentIo::init()
{
    QVERIFY(m_dir.isValid());
}

void TestDocumentIo::writeThenLoadRoundTrips()
{
    QTextEdit edit;
    DocumentIo io(&edit);
    edit.setMarkdown(QStringLiteral("# Hola\n\ntexto **negrita**"));

    const QString path = pathFor(QStringLiteral("doc.md"));
    QString error;
    QVERIFY2(io.write(path, &error), qPrintable(error));
    QCOMPARE(io.currentFile(), path);

    // Cargar en otro editor reproduce el contenido.
    QTextEdit edit2;
    DocumentIo io2(&edit2);
    QVERIFY2(io2.load(path, &error), qPrintable(error));
    const QString plain = edit2.toPlainText();
    QVERIFY(plain.contains(QStringLiteral("Hola")));
    QVERIFY(plain.contains(QStringLiteral("negrita")));
}

void TestDocumentIo::loadMissingFileFails()
{
    QTextEdit edit;
    DocumentIo io(&edit);
    QString error;
    QVERIFY(!io.load(pathFor(QStringLiteral("no-existe.md")), &error));
    QVERIFY(!error.isEmpty());
}

void TestDocumentIo::writeUpdatesCurrentFileAndClearsModified()
{
    QTextEdit edit;
    DocumentIo io(&edit);
    // Insertar con el cursor simula una edición real (marca el documento como
    // modificado; setPlainText() no lo haría).
    edit.textCursor().insertText(QStringLiteral("contenido"));
    QVERIFY(edit.document()->isModified());

    const QString path = pathFor(QStringLiteral("guardado.md"));
    QString error;
    QVERIFY2(io.write(path, &error), qPrintable(error));
    QCOMPARE(io.currentFile(), path);
    QVERIFY(!io.isModified());  // guardar limpia el estado modificado
}

void TestDocumentIo::isModifiedTracksContentAgainstBaseline()
{
    // Prepara un archivo y cárgalo: fija la línea base.
    const QString path = pathFor(QStringLiteral("base.md"));
    QString error;
    {
        QTextEdit writer;
        DocumentIo writerIo(&writer);
        writer.setMarkdown(QStringLiteral("# Hola"));
        QVERIFY2(writerIo.write(path, &error), qPrintable(error));
    }

    QTextEdit edit;
    DocumentIo io(&edit);
    QVERIFY2(io.load(path, &error), qPrintable(error));
    QVERIFY(!io.isModified());  // recién cargado: limpio

    edit.textCursor().insertText(QStringLiteral("texto nuevo"));
    QVERIFY(io.isModified());   // hay un cambio real de contenido

    edit.undo();
    QVERIFY(!io.isModified());  // de vuelta al contenido base => limpio
}

void TestDocumentIo::loadFromStringIsUntitledModifiedAndKeepsFrontMatter()
{
    QTextEdit edit;
    DocumentIo io(&edit);

    QSignalSpy fileSpy(&io, &DocumentIo::currentFileChanged);
    QSignalSpy loadSpy(&io, &DocumentIo::documentLoaded);
    io.loadFromString(QStringLiteral("---\n"
                                     "title: Plantilla\n"
                                     "---\n"
                                     "\n"
                                     "# CERTIFICO:\n"
                                     "\n"
                                     "Cuerpo de la plantilla.\n"));

    // Sin archivo asociado: «Guardar» pedirá nombre.
    QCOMPARE(io.currentFile(), QString());
    // Cuenta como modificado para no perder la plantilla sin avisar.
    QVERIFY(io.isModified());
    // Front matter conservado y no renderizado.
    QVERIFY(io.hasFrontMatter());
    const QString plain = edit.toPlainText();
    QVERIFY(plain.contains(QStringLiteral("CERTIFICO")));
    QVERIFY(!plain.contains(QStringLiteral("title:")));
    // Emite las mismas señales que una carga (para refrescar vista e índice).
    QCOMPARE(fileSpy.count(), 1);
    QCOMPARE(loadSpy.count(), 1);
}

void TestDocumentIo::loadFromStringNotModifiedWhenAsModifiedFalse()
{
    // El documento de bienvenida se carga con asModified=false: NO debe contar como
    // modificado (la línea base es su propio contenido), así cerrar no da la lata.
    QTextEdit edit;
    DocumentIo io(&edit);
    io.loadFromString(QStringLiteral("# Bienvenido\n\nTexto de bienvenida.\n"), false);
    QCOMPARE(io.currentFile(), QString());
    QVERIFY(!io.isModified());
    QVERIFY(edit.toPlainText().contains(QStringLiteral("Bienvenido")));
}

void TestDocumentIo::frontMatterIsPreserved()
{
    const QString input =
        QStringLiteral("---\n"
                       "title: Mi nota\n"
                       "tags: [a, b]\n"
                       "---\n"
                       "\n"
                       "# Encabezado\n"
                       "\n"
                       "Cuerpo del documento.\n");
    const QString path = writeRaw(QStringLiteral("confm.md"), input);

    QTextEdit edit;
    DocumentIo io(&edit);
    QString error;
    QVERIFY2(io.load(path, &error), qPrintable(error));

    QVERIFY(io.hasFrontMatter());
    // El front matter no se renderiza: no aparece en el documento.
    const QString plain = edit.toPlainText();
    QVERIFY(plain.contains(QStringLiteral("Encabezado")));
    QVERIFY(plain.contains(QStringLiteral("Cuerpo del documento")));
    QVERIFY(!plain.contains(QStringLiteral("title:")));
    QVERIFY(!plain.contains(QStringLiteral("tags:")));

    // Al guardar, el front matter se reescribe verbatim al principio.
    const QString outPath = pathFor(QStringLiteral("salida.md"));
    QVERIFY2(io.write(outPath, &error), qPrintable(error));
    const QString written = readRaw(outPath);
    QVERIFY(written.startsWith(
        QStringLiteral("---\ntitle: Mi nota\ntags: [a, b]\n---\n")));
    QVERIFY(written.contains(QStringLiteral("Cuerpo del documento")));
}

void TestDocumentIo::noFrontMatterWhenNotAtStart()
{
    // Un '---' que no está al principio es una regla horizontal, no front matter.
    const QString input =
        QStringLiteral("# Título\n\nTexto\n\n---\n\nMás texto\n");
    const QString path = writeRaw(QStringLiteral("sinfm.md"), input);

    QTextEdit edit;
    DocumentIo io(&edit);
    QString error;
    QVERIFY2(io.load(path, &error), qPrintable(error));
    QVERIFY(!io.hasFrontMatter());
    QVERIFY(edit.toPlainText().contains(QStringLiteral("Título")));
}

void TestDocumentIo::resetClearsAndSignals()
{
    QTextEdit edit;
    DocumentIo io(&edit);
    edit.setMarkdown(QStringLiteral("algo"));

    QSignalSpy spy(&io, &DocumentIo::currentFileChanged);
    io.reset();

    QVERIFY(edit.toPlainText().isEmpty());
    QCOMPARE(io.currentFile(), QString());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().toString(), QString());
}

void TestDocumentIo::loadEmitsDocumentLoaded()
{
    QTextEdit writer;
    DocumentIo writerIo(&writer);
    writer.setMarkdown(QStringLiteral("[enlace](https://example.com)"));
    const QString path = pathFor(QStringLiteral("conlink.md"));
    QString error;
    QVERIFY2(writerIo.write(path, &error), qPrintable(error));

    QTextEdit edit;
    DocumentIo io(&edit);
    QSignalSpy loadedSpy(&io, &DocumentIo::documentLoaded);
    QSignalSpy fileSpy(&io, &DocumentIo::currentFileChanged);
    QVERIFY2(io.load(path, &error), qPrintable(error));
    QCOMPARE(loadedSpy.count(), 1);
    QCOMPARE(fileSpy.count(), 1);
}

QTEST_MAIN(TestDocumentIo)
#include "tst_documentio.moc"
