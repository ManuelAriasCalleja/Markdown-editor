#include <QtTest>

#include <QApplication>
#include <QCoreApplication>
#include <QFile>
#include <QSettings>
#include <QTemporaryDir>
#include <QTextEdit>
#include <QTextStream>

#include "documentio.h"
#include "filecontroller.h"
#include "focuseditor.h"
#include "mainwindow.h"
#include "splitviewcontroller.h"
#include "tableedit.h"

// Pruebas de caracterización de las operaciones de archivo de MainWindow
// (guardar, abrir, cuerpo actual). Fijan el comportamiento observable —contenido
// escrito a disco, estado «modificado», contenido cargado, fuente de
// `currentBody`— ANTES de extraer la lógica a FileController, para demostrar que
// la refactorización no cambia la conducta. Evitan los diálogos modales (no
// modifican antes de abrir, no llaman a maybeSave con cambios). Es friend de
// MainWindow para invocar las operaciones privadas y leer los colaboradores.
class TestFileController : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void writeToFileRoundTrips();
    void openFileLoadsContent();
    void currentBodyComesFromDocument();
    void currentBodyComesFromSourcePanel();
    void saveToExistingFileUpdatesDisk();

private:
    static QString readFile(const QString &path);
};

void TestFileController::initTestCase()
{
    QCoreApplication::setOrganizationName(QStringLiteral("md-editor-test"));
    QCoreApplication::setApplicationName(QStringLiteral("md-editor-test"));
}

void TestFileController::cleanup()
{
    QSettings().clear();
}

QString TestFileController::readFile(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();
    QTextStream in(&f);
    in.setEncoding(QStringConverter::Utf8);
    return in.readAll();
}

void TestFileController::writeToFileRoundTrips()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("out.md"));

    MainWindow w;
    w.m_editor->setMarkdown(QStringLiteral("# Título\n\nUn párrafo.\n"));
    QVERIFY(w.m_file->writeToFile(path));

    // El archivo contiene la serialización canónica del documento.
    QCOMPARE(readFile(path), mdtable::documentMarkdown(w.m_editor->document()));
    // Tras guardar, el documento ya no está modificado respecto a lo escrito.
    QVERIFY(!w.m_documentIo->isModified());
    QCOMPARE(w.m_documentIo->currentFile(), path);
}

void TestFileController::openFileLoadsContent()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("in.md"));
    {
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
        QTextStream out(&f);
        out.setEncoding(QStringConverter::Utf8);
        out << QStringLiteral("# Encabezado\n\nCuerpo del documento.\n");
    }

    MainWindow w;  // documento nuevo: no modificado, openFile no pregunta
    w.m_file->openFile(path);
    QCOMPARE(w.m_documentIo->currentFile(), path);
    const QString md = mdtable::documentMarkdown(w.m_editor->document());
    QVERIFY(md.contains(QStringLiteral("Encabezado")));
    QVERIFY(md.contains(QStringLiteral("Cuerpo del documento")));
}

void TestFileController::currentBodyComesFromDocument()
{
    MainWindow w;
    w.m_editor->setMarkdown(QStringLiteral("texto plano\n"));
    // Sin panel de fuente sucio: el cuerpo se serializa del documento WYSIWYG.
    QCOMPARE(w.m_file->currentBody(), mdtable::documentMarkdown(w.m_editor->document()));
}

void TestFileController::currentBodyComesFromSourcePanel()
{
    MainWindow w;
    w.m_split->toggleSplitView(true);
    w.m_split->sourceEditor()->setPlainText(QStringLiteral("# Desde el fuente\n"));
    // Con el fuente sucio, currentBody toma su texto literal.
    QVERIFY(w.m_split->isSourceDirty());
    QCOMPARE(w.m_file->currentBody(), QStringLiteral("# Desde el fuente\n"));
}

void TestFileController::saveToExistingFileUpdatesDisk()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("doc.md"));

    MainWindow w;
    w.m_editor->setMarkdown(QStringLiteral("versión 1\n"));
    QVERIFY(w.m_file->writeToFile(path));

    // Edita y guarda con save() (usa el archivo actual, sin diálogo).
    w.m_editor->setMarkdown(QStringLiteral("versión 2 corregida\n"));
    QVERIFY(w.m_file->save());
    QVERIFY(readFile(path).contains(QStringLiteral("versión 2 corregida")));
    QVERIFY(!w.m_documentIo->isModified());
}

QTEST_MAIN(TestFileController)
#include "tst_filecontroller.moc"
