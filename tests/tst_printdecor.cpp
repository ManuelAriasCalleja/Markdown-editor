#include <QtTest>

#include <QByteArray>
#include <QFile>
#include <QPrinter>
#include <QTemporaryDir>
#include <QTextDocument>

#include "exporters.h"
#include "printdecor.h"

// Pruebas de la decoración de impresión: el texto puro del número de página y un
// smoke test de la paginación a mano (paintPaginated) sobre un PDF real.
class TestPrintDecor : public QObject
{
    Q_OBJECT
private slots:
    void pageNumberTextIsNeutral();
    void paintPaginatedProducesMultiPagePdf();
    void paintPaginatedWithoutFooterAlsoWorks();

private:
    // Nº de páginas de un PDF, contando los objetos «/Type /Page» (no /Pages).
    static int pdfPageCount(const QString &path);
    // Documento largo (muchos párrafos) que ocupa varias páginas.
    static QString longMarkdown();
};

QString TestPrintDecor::longMarkdown()
{
    QString md;
    for (int i = 0; i < 400; ++i)
        md += QStringLiteral("Párrafo número %1 con texto suficiente para llenar la línea.\n\n").arg(i);
    return md;
}

int TestPrintDecor::pdfPageCount(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return -1;
    const QByteArray data = f.readAll();
    // Cuenta los objetos de página: "/Type /Page" seguido de un no-'s' (para no
    // contar "/Type /Pages"). El árbol de páginas de Qt no comprime esta parte.
    int count = 0;
    int idx = 0;
    const QByteArray needle = "/Type /Page";
    while ((idx = data.indexOf(needle, idx)) != -1) {
        const int after = idx + needle.size();
        if (after >= data.size() || data.at(after) != 's')
            ++count;
        idx = after;
    }
    return count;
}

void TestPrintDecor::pageNumberTextIsNeutral()
{
    QCOMPARE(mdprintdecor::pageNumberText(1, 3), QStringLiteral("1 / 3"));
    QCOMPARE(mdprintdecor::pageNumberText(12, 12), QStringLiteral("12 / 12"));
}

void TestPrintDecor::paintPaginatedProducesMultiPagePdf()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("out.pdf"));

    QTextDocument doc;
    doc.setMarkdown(longMarkdown());

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(path);
    mdexport::paintPaginated(&printer, &doc, /*footerPageNumbers=*/true);

    QVERIFY(QFile::exists(path));
    QVERIFY(QFileInfo(path).size() > 0);
    // Un documento tan largo ocupa claramente más de una página.
    QVERIFY2(pdfPageCount(path) > 1,
             qPrintable(QStringLiteral("páginas=%1").arg(pdfPageCount(path))));
}

void TestPrintDecor::paintPaginatedWithoutFooterAlsoWorks()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("nofooter.pdf"));

    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("Un documento corto.\n"));

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(path);
    mdexport::paintPaginated(&printer, &doc, /*footerPageNumbers=*/false);

    QVERIFY(QFile::exists(path));
    QCOMPARE(pdfPageCount(path), 1);
}

QTEST_MAIN(TestPrintDecor)
#include "tst_printdecor.moc"
