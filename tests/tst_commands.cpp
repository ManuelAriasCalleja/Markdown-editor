#include <QtTest>

#include <QAction>
#include <QMenu>
#include <QMenuBar>

#include "commandpalettedialog.h"

// Pruebas de la lógica pura de la paleta de comandos (mdcommands): recolección
// desde un QMenuBar, coincidencia difusa y filtrado/ordenación.
class TestCommands : public QObject
{
    Q_OBJECT
private slots:
    void fuzzyMatchSubsequence();
    void fuzzyMatchCaseInsensitive();
    void fuzzyMatchEmptyQuery();
    void fuzzyMatchNoMatch();
    void fuzzyScorePrefersWordStart();
    void filterEmptyReturnsAll();
    void filterRanksByScore();
    void filterNoMatchIsEmpty();
    void collectBuildsBreadcrumbPaths();
    void collectSkipsSeparatorsAndDisabled();
    void collectStripsMnemonics();
    void collectCapturesShortcut();
};

void TestCommands::fuzzyMatchSubsequence()
{
    QVERIFY(mdcommands::fuzzyMatch(QStringLiteral("Guardar como"), QStringLiteral("gc")));
    QVERIFY(mdcommands::fuzzyMatch(QStringLiteral("Guardar como"), QStringLiteral("guar")));
    QVERIFY(!mdcommands::fuzzyMatch(QStringLiteral("Guardar como"), QStringLiteral("cg")));
}

void TestCommands::fuzzyMatchCaseInsensitive()
{
    QVERIFY(mdcommands::fuzzyMatch(QStringLiteral("Paleta de Comandos"), QStringLiteral("PALETA")));
    QVERIFY(mdcommands::fuzzyMatch(QStringLiteral("Paleta de Comandos"), QStringLiteral("cmd")));
}

void TestCommands::fuzzyMatchEmptyQuery()
{
    int score = -1;
    QVERIFY(mdcommands::fuzzyMatch(QStringLiteral("cualquiera"), QString(), &score));
    QCOMPARE(score, 0);
}

void TestCommands::fuzzyMatchNoMatch()
{
    int score = -1;
    QVERIFY(!mdcommands::fuzzyMatch(QStringLiteral("Exportar"), QStringLiteral("zzz"), &score));
    QCOMPARE(score, 0);
}

void TestCommands::fuzzyScorePrefersWordStart()
{
    // "ir" a inicio de dos palabras ("Ir a") puntúa más que embebido ("Salir").
    int start = 0;
    int mid = 0;
    QVERIFY(mdcommands::fuzzyMatch(QStringLiteral("Ir a línea"), QStringLiteral("ir"), &start));
    QVERIFY(mdcommands::fuzzyMatch(QStringLiteral("Salir"), QStringLiteral("ir"), &mid));
    QVERIFY(start > mid);
}

void TestCommands::filterEmptyReturnsAll()
{
    QList<mdcommands::Command> cmds;
    cmds.append({QStringLiteral("Archivo › Guardar"), QString(), nullptr});
    cmds.append({QStringLiteral("Editar › Copiar"), QString(), nullptr});
    const QList<mdcommands::Command> out = mdcommands::filterCommands(cmds, QStringLiteral("   "));
    QCOMPARE(out.size(), 2);
    QCOMPARE(out.at(0).path, QStringLiteral("Archivo › Guardar"));
    QCOMPARE(out.at(1).path, QStringLiteral("Editar › Copiar"));
}

void TestCommands::filterRanksByScore()
{
    QList<mdcommands::Command> cmds;
    cmds.append({QStringLiteral("Insertar › Imagen"), QString(), nullptr});
    cmds.append({QStringLiteral("Guardar como"), QString(), nullptr});    // 'como' embebido
    cmds.append({QStringLiteral("Comandos"), QString(), nullptr});        // inicio de palabra
    const QList<mdcommands::Command> out = mdcommands::filterCommands(cmds, QStringLiteral("com"));
    QCOMPARE(out.size(), 2);
    // "Comandos" (inicio de palabra) debe salir por delante de "Guardar como".
    QCOMPARE(out.at(0).path, QStringLiteral("Comandos"));
    QCOMPARE(out.at(1).path, QStringLiteral("Guardar como"));
}

void TestCommands::filterNoMatchIsEmpty()
{
    QList<mdcommands::Command> cmds;
    cmds.append({QStringLiteral("Archivo › Guardar"), QString(), nullptr});
    QVERIFY(mdcommands::filterCommands(cmds, QStringLiteral("xyz")).isEmpty());
}

void TestCommands::collectBuildsBreadcrumbPaths()
{
    QMenuBar bar;
    QMenu *file = bar.addMenu(QStringLiteral("Archivo"));
    file->addAction(QStringLiteral("Guardar"));
    QMenu *exp = file->addMenu(QStringLiteral("Exportar"));
    exp->addAction(QStringLiteral("A texto plano"));

    const QList<mdcommands::Command> cmds = mdcommands::collectCommands(&bar);
    QStringList paths;
    for (const mdcommands::Command &c : cmds)
        paths << c.path;
    QVERIFY(paths.contains(QStringLiteral("Archivo › Guardar")));
    QVERIFY(paths.contains(QStringLiteral("Archivo › Exportar › A texto plano")));
    // Cada comando conserva el puntero a su acción.
    for (const mdcommands::Command &c : cmds)
        QVERIFY(c.action != nullptr);
}

void TestCommands::collectSkipsSeparatorsAndDisabled()
{
    QMenuBar bar;
    QMenu *file = bar.addMenu(QStringLiteral("Archivo"));
    file->addAction(QStringLiteral("Nuevo"));
    file->addSeparator();
    QAction *off = file->addAction(QStringLiteral("Deshabilitada"));
    off->setEnabled(false);

    const QList<mdcommands::Command> cmds = mdcommands::collectCommands(&bar);
    QStringList paths;
    for (const mdcommands::Command &c : cmds)
        paths << c.path;
    QVERIFY(paths.contains(QStringLiteral("Archivo › Nuevo")));
    QVERIFY(!paths.contains(QStringLiteral("Archivo › Deshabilitada")));
    QCOMPARE(cmds.size(), 1);  // ni el separador ni la deshabilitada cuentan
}

void TestCommands::collectStripsMnemonics()
{
    QMenuBar bar;
    QMenu *file = bar.addMenu(QStringLiteral("&Archivo"));
    file->addAction(QStringLiteral("&Guardar"));

    const QList<mdcommands::Command> cmds = mdcommands::collectCommands(&bar);
    QCOMPARE(cmds.size(), 1);
    QCOMPARE(cmds.at(0).path, QStringLiteral("Archivo › Guardar"));  // sin '&'
}

void TestCommands::collectCapturesShortcut()
{
    QMenuBar bar;
    QMenu *file = bar.addMenu(QStringLiteral("Archivo"));
    QAction *save = file->addAction(QStringLiteral("Guardar"));
    save->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));

    const QList<mdcommands::Command> cmds = mdcommands::collectCommands(&bar);
    QCOMPARE(cmds.size(), 1);
    QCOMPARE(cmds.at(0).shortcut,
             QKeySequence(Qt::CTRL | Qt::Key_S).toString(QKeySequence::NativeText));
}

QTEST_MAIN(TestCommands)
#include "tst_commands.moc"
