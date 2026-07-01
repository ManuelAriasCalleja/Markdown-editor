#include <QtTest>

#include <QLabel>
#include <QLineEdit>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>

#include "find.h"
#include "findreplacebar.h"

// Pruebas de la búsqueda pura (mdfind): recuento/localización de coincidencias,
// ordinal «N de M» y —como camino único que usa «Reemplazar todo»— la aplicación
// de reemplazos de atrás hacia adelante sobre un QTextDocument real.
class TestFind : public QObject
{
    Q_OBJECT
private slots:
    void literalFindsAll();
    void caseInsensitiveByDefault();
    void caseSensitiveRespected();
    void wholeWordOnly();
    void regexMode();
    void emptyNeedleNoMatches();
    void invalidRegexNoMatches();
    void skipsEmptyMatches();
    void ordinalByStart();
    void replaceAllBackToFrontMatchesDocument();

    // Integración de la barra: contador «N de M», señal de resaltado y Reemplazar todo.
    void barCountLabelShowsOrdinal();
    void barEmitsHighlightMatches();
    void barReplaceAllReplacesAndRecounts();
};

void TestFind::literalFindsAll()
{
    const auto m = mdfind::findAll(QStringLiteral("a.b.a.b"), QStringLiteral("a"),
                                   false, false, false);
    QCOMPARE(m.size(), 2);
    QCOMPARE(m.at(0).start, 0);
    QCOMPARE(m.at(0).length, 1);
    QCOMPARE(m.at(1).start, 4);
    // El punto es literal (no metacarácter) en modo no-regex.
    QCOMPARE(mdfind::findAll(QStringLiteral("axb"), QStringLiteral("a.b"), false, false, false)
                 .size(),
             0);
}

void TestFind::caseInsensitiveByDefault()
{
    QCOMPARE(mdfind::findAll(QStringLiteral("Hola HOLA hola"), QStringLiteral("hola"),
                             false, false, false)
                 .size(),
             3);
}

void TestFind::caseSensitiveRespected()
{
    QCOMPARE(mdfind::findAll(QStringLiteral("Hola HOLA hola"), QStringLiteral("hola"),
                             false, true, false)
                 .size(),
             1);
}

void TestFind::wholeWordOnly()
{
    // "sol" como palabra: casa "sol" pero no "solo" ni "girasol".
    const auto m = mdfind::findAll(QStringLiteral("sol solo girasol sol"),
                                   QStringLiteral("sol"), false, false, true);
    QCOMPARE(m.size(), 2);
    QCOMPARE(m.at(0).start, 0);
    QCOMPARE(m.at(1).start, 17);
}

void TestFind::regexMode()
{
    const auto m = mdfind::findAll(QStringLiteral("a1 b22 c333"), QStringLiteral("[a-z][0-9]+"),
                                   true, false, false);
    QCOMPARE(m.size(), 3);
    QCOMPARE(m.at(1).length, 3);  // "b22"
}

void TestFind::emptyNeedleNoMatches()
{
    QVERIFY(mdfind::findAll(QStringLiteral("texto"), QString(), false, false, false).isEmpty());
}

void TestFind::invalidRegexNoMatches()
{
    QVERIFY(mdfind::findAll(QStringLiteral("texto"), QStringLiteral("[unclosed"),
                            true, false, false)
                .isEmpty());
}

void TestFind::skipsEmptyMatches()
{
    // "a*" casa cadenas vacías por todas partes; solo cuentan las no vacías.
    const auto m = mdfind::findAll(QStringLiteral("baab"), QStringLiteral("a*"),
                                   true, false, false);
    QCOMPARE(m.size(), 1);
    QCOMPARE(m.at(0).start, 1);
    QCOMPARE(m.at(0).length, 2);
}

void TestFind::ordinalByStart()
{
    const auto m = mdfind::findAll(QStringLiteral("a.a.a"), QStringLiteral("a"),
                                   false, false, false);
    QCOMPARE(mdfind::matchOrdinal(m, 0), 1);
    QCOMPARE(mdfind::matchOrdinal(m, 2), 2);
    QCOMPARE(mdfind::matchOrdinal(m, 4), 3);
    QCOMPARE(mdfind::matchOrdinal(m, 1), 0);  // el cursor no está sobre una coincidencia
    QCOMPARE(mdfind::matchOrdinal({}, 0), 0);
}

void TestFind::replaceAllBackToFrontMatchesDocument()
{
    // Verifica el mapeo offset-de-texto-plano → posición-de-documento y el
    // reemplazo de atrás hacia adelante (lo que hará FindReplaceBar::replaceAll).
    QTextDocument doc;
    doc.setPlainText(QStringLiteral("aXaXa"));
    const auto matches = mdfind::findAll(doc.toPlainText(), QStringLiteral("a"),
                                         false, false, false);
    QCOMPARE(matches.size(), 3);
    QTextCursor group(&doc);
    group.beginEditBlock();
    for (int i = matches.size() - 1; i >= 0; --i) {
        QTextCursor c(&doc);
        c.setPosition(matches.at(i).start);
        c.setPosition(matches.at(i).start + matches.at(i).length, QTextCursor::KeepAnchor);
        c.insertText(QStringLiteral("bb"));
    }
    group.endEditBlock();
    QCOMPARE(doc.toPlainText(), QStringLiteral("bbXbbXbb"));
}

void TestFind::barCountLabelShowsOrdinal()
{
    QTextEdit ed;
    ed.setPlainText(QStringLiteral("a a a"));  // 3 coincidencias de "a", cursor en 0
    FindReplaceBar bar(&ed);
    bar.m_findEdit->setText(QStringLiteral("a"));  // dispara updateMatches
    // El cursor está en la posición 0, que es el inicio de la 1.ª coincidencia.
    QCOMPARE(bar.m_countLabel->text(), QStringLiteral("1 de 3"));
    // Sin término: el contador se limpia.
    bar.m_findEdit->clear();
    QVERIFY(bar.m_countLabel->text().isEmpty());
}

void TestFind::barEmitsHighlightMatches()
{
    QTextEdit ed;
    ed.setPlainText(QStringLiteral("uno dos uno"));
    FindReplaceBar bar(&ed);
    QList<mdfind::Match> captured;
    bool emitted = false;
    QObject::connect(&bar, &FindReplaceBar::highlightMatches, &bar,
                     [&](const QList<mdfind::Match> &m) { captured = m; emitted = true; });
    bar.m_findEdit->setText(QStringLiteral("uno"));
    QVERIFY(emitted);
    QCOMPARE(captured.size(), 2);
}

void TestFind::barReplaceAllReplacesAndRecounts()
{
    QTextEdit ed;
    ed.setPlainText(QStringLiteral("a a a"));
    FindReplaceBar bar(&ed);
    bar.m_findEdit->setText(QStringLiteral("a"));
    bar.m_replaceEdit->setText(QStringLiteral("bb"));
    bar.replaceAll();
    QCOMPARE(ed.toPlainText(), QStringLiteral("bb bb bb"));
    // Ya no queda "a": el contador lo refleja.
    QCOMPARE(bar.m_countLabel->text(), QStringLiteral("Sin coincidencias"));
}

QTEST_MAIN(TestFind)
#include "tst_find.moc"
