#include <QtTest>

#include "htmlimport.h"
#include "richpaste.h"

// Pruebas de la importación de HTML: la decodificación charset-aware (mdimport) y la
// conversión completa a Markdown (reusando mdrichpaste::htmlToMarkdown).
class TestHtmlImport : public QObject
{
    Q_OBJECT
private slots:
    void charsetFromMetaCharset();
    void charsetFromHttpEquiv();
    void charsetAbsentIsEmpty();
    void decodesUtf8ByDefault();
    void decodesLatin1WhenDeclared();
    void decodesUtf8Bom();
    void convertsSimpleHtmlToMarkdown();
};

void TestHtmlImport::charsetFromMetaCharset()
{
    QCOMPARE(mdimport::charsetOf("<html><head><meta charset=\"ISO-8859-1\"></head>"),
             QStringLiteral("iso-8859-1"));
    // Sin comillas y con espacios también.
    QCOMPARE(mdimport::charsetOf("<meta charset = utf-8 >"), QStringLiteral("utf-8"));
}

void TestHtmlImport::charsetFromHttpEquiv()
{
    QCOMPARE(mdimport::charsetOf(
                 "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">"),
             QStringLiteral("utf-8"));
}

void TestHtmlImport::charsetAbsentIsEmpty()
{
    QVERIFY(mdimport::charsetOf("<html><head><title>x</title></head>").isEmpty());
}

void TestHtmlImport::decodesUtf8ByDefault()
{
    // «Título» en UTF-8, sin declaración: debe decodificarse como UTF-8.
    const QByteArray bytes = QString::fromUtf8("<p>Título</p>").toUtf8();
    QCOMPARE(mdimport::decodeHtml(bytes), QStringLiteral("<p>Título</p>"));
}

void TestHtmlImport::decodesLatin1WhenDeclared()
{
    // «ó» en Latin-1 es el byte 0xF3; declarado el charset, debe salir «ó», no basura.
    QByteArray bytes = "<meta charset=\"iso-8859-1\"><p>a";
    bytes += char(0xF3);  // ó en Latin-1
    bytes += "b</p>";
    const QString decoded = mdimport::decodeHtml(bytes);
    QVERIFY2(decoded.contains(QStringLiteral("aób")), qPrintable(decoded));
}

void TestHtmlImport::decodesUtf8Bom()
{
    QByteArray bytes;
    bytes += char(0xEF);
    bytes += char(0xBB);
    bytes += char(0xBF);  // BOM UTF-8
    bytes += QString::fromUtf8("<p>ñ</p>").toUtf8();
    const QString decoded = mdimport::decodeHtml(bytes);
    QVERIFY(decoded.contains(QStringLiteral("ñ")));
    QVERIFY(!decoded.startsWith(QChar(0xFEFF)));  // el BOM se consume, no queda en el texto
}

void TestHtmlImport::convertsSimpleHtmlToMarkdown()
{
    // Extremo a extremo: bytes HTML → texto → Markdown.
    const QByteArray bytes = QString::fromUtf8(
                                 "<h1>Título</h1><p>Un <strong>párrafo</strong> con "
                                 "<a href=\"https://ej.com\">enlace</a>.</p>")
                                 .toUtf8();
    const QString md = mdrichpaste::htmlToMarkdown(mdimport::decodeHtml(bytes));
    QVERIFY2(md.contains(QStringLiteral("# Título")), qPrintable(md));
    QVERIFY2(md.contains(QStringLiteral("**párrafo**")), qPrintable(md));
    QVERIFY2(md.contains(QStringLiteral("[enlace](https://ej.com)")), qPrintable(md));
}

QTEST_MAIN(TestHtmlImport)
#include "tst_htmlimport.moc"
