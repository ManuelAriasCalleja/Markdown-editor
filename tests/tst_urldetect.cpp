#include <QtTest>

#include "urldetect.h"

class TestUrlDetect : public QObject
{
    Q_OBJECT
private slots:
    void acceptsSchemedUrls();
    void rejectsNonUrls();
    void trimsSurroundingSpace();
};

void TestUrlDetect::acceptsSchemedUrls()
{
    QVERIFY(mdurl::looksLikeUrl(QStringLiteral("https://example.com")));
    QVERIFY(mdurl::looksLikeUrl(QStringLiteral("http://a.b/c?d=1&e=2")));
    QVERIFY(mdurl::looksLikeUrl(QStringLiteral("HTTPS://EXAMPLE.COM")));  // sin distinción de caso
    QVERIFY(mdurl::looksLikeUrl(QStringLiteral("ftp://files.example.org/x")));
    QVERIFY(mdurl::looksLikeUrl(QStringLiteral("mailto:user@example.com")));
}

void TestUrlDetect::rejectsNonUrls()
{
    QVERIFY(!mdurl::looksLikeUrl(QString()));
    QVERIFY(!mdurl::looksLikeUrl(QStringLiteral("hola mundo")));
    QVERIFY(!mdurl::looksLikeUrl(QStringLiteral("www.example.com")));      // sin esquema
    QVERIFY(!mdurl::looksLikeUrl(QStringLiteral("http://a b")));           // espacio interno
    QVERIFY(!mdurl::looksLikeUrl(QStringLiteral("texto http://x.y final")));
    QVERIFY(!mdurl::looksLikeUrl(QStringLiteral("example.com")));
}

void TestUrlDetect::trimsSurroundingSpace()
{
    QVERIFY(mdurl::looksLikeUrl(QStringLiteral("  https://example.com \n")));
}

QTEST_MAIN(TestUrlDetect)
#include "tst_urldetect.moc"
